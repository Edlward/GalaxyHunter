#include "gphoto_camera.h"
#include "gphoto_commons.h"
#include <QTemporaryFile>
#include "utils/scope.h"
#include <QThread>
#include <QImage>
#include <QDebug>

#include <GraphicsMagick/Magick++.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
class GPhotoCamera::Private {
public:
  Private(const std::shared_ptr<GPhotoCameraInformation> &info)
    : model(QString::fromStdString(info->name)), port(info->port), context(info->context) {}
  std::string port;
  QString model;
  QString about;
  QString summary;
  GPContext* context;
  Camera *camera = nullptr;
};



struct CameraTempFile {
  CameraTempFile();
  ~CameraTempFile();
  int save();
  CameraFile *camera_file;
  QTemporaryFile temp_file;
  operator CameraFile *() const { return camera_file; }
  operator QString() const { return path(); }
  QString mimeType() const;
  QString path() const { return temp_file.fileName(); }
};




GPhotoCamera::GPhotoCamera(const shared_ptr< GPhotoCameraInformation > &gphotoCameraInformation)
  : d(new Private{gphotoCameraInformation})
{
  gp_api{{
    { [=] { return gp_camera_new(&d->camera); } },
  }}.on_error([=](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << errorMessage;
    emit error(this, QString::fromLocal8Bit(errorMessage));
  });
}

struct CameraSetting : enable_shared_from_this<CameraSetting> {
  int id;
  string name;
  string label;
  string info;
  CameraWidgetType type;
  string path() const;
  vector<shared_ptr<CameraSetting>> children;
  shared_ptr<CameraSetting> parent;
  static shared_ptr<CameraSetting> from(CameraWidget *widget, const shared_ptr<CameraSetting> &parent);
};

// TODO: move to utils?
QDebug &operator<<(QDebug &d, const std::string &s) {
  d << QString::fromStdString(s);
  return d;
}

string CameraSetting::path() const
{
  boost::filesystem::path p;
  auto v = shared_from_this();
  while(v->parent) {
    p = boost::filesystem::path(v->parent->name) / p;
    v = v->parent;
  }
  return p.string();
}


shared_ptr< CameraSetting > CameraSetting::from(CameraWidget* widget, const shared_ptr< CameraSetting >& parent = {})
{
  const char *s;
  auto setting = make_shared<CameraSetting>();
  setting->parent = parent;
  auto to_string = [&](string &dest, int error_code) {
    if(error_code == GP_OK)
      dest = {s};
    return error_code;
  };
  gp_api{{
    sequence_run( [&]{ return gp_widget_get_id(widget, &setting->id); }),
    sequence_run( [&]{ return to_string(setting->info, gp_widget_get_info(widget, &s) ); }),
    sequence_run( [&]{ return to_string(setting->label, gp_widget_get_label(widget, &s) ); }),
    sequence_run( [&]{ return to_string(setting->name, gp_widget_get_name(widget, &s) ); }),
    sequence_run( [&]{ return gp_widget_get_type(widget, &setting->type); }),
  }}.on_error([&](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << "Error decoding setting on " << label << ": " << errorMessage;
  });
  for(int i=0; i<gp_widget_count_children(widget); i++) {
    CameraWidget *child;
    // TODO scope cleanup_child{[&child]{gp_widget_unref(child);}};
    if(gp_widget_get_child(widget, i, &child) == GP_OK)
      setting->children.push_back(from(child, setting));
  }
  return setting;
}


void GPhotoCamera::connect()
{
  CameraAbilities abilities;
  GPPortInfo portInfo;
  CameraAbilitiesList *abilities_list = nullptr;
  GPPortInfoList *portInfoList = nullptr;
  CameraText camera_summary;
  CameraText camera_about;
  int model, port;
  gp_api{{
    sequence_run( [&]{ return gp_abilities_list_new (&abilities_list); } ),
    sequence_run( [&]{ return gp_abilities_list_load(abilities_list, d->context); } ),
    sequence_run( [&]{ model = gp_abilities_list_lookup_model(abilities_list, d->model.toLocal8Bit()); return model; } ),
    sequence_run( [&]{ return gp_abilities_list_get_abilities(abilities_list, model, &abilities); } ),
    sequence_run( [&]{ return gp_camera_set_abilities(d->camera, abilities); } ),
    sequence_run( [&]{ return gp_port_info_list_new(&portInfoList); } ),
    sequence_run( [&]{ return gp_port_info_list_load(portInfoList); } ),
    sequence_run( [&]{ return gp_port_info_list_count(portInfoList); } ),
    sequence_run( [&]{ port = gp_port_info_list_lookup_path(portInfoList, d->port.c_str()); return port; } ),
    sequence_run( [&]{ return gp_port_info_list_get_info(portInfoList, port, &portInfo); return port; } ),
    sequence_run( [&]{ return gp_camera_set_port_info(d->camera, portInfo); } ),
    sequence_run( [&]{ return gp_camera_get_summary(d->camera, &camera_summary, d->context); } ),
    sequence_run( [&]{ return gp_camera_get_about(d->camera, &camera_about, d->context); } ),
  }}.on_error([=](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << "on " << label << ": " << errorMessage;
    emit error(this, QString::fromLocal8Bit(errorMessage));
  }).run_last([&]{
    d->summary = QString(camera_summary.text);
    d->about = QString(camera_about.text);
    emit connected();    
  });
  CameraWidget *settings;
  if(gp_camera_get_config(d->camera, &settings, d->context) == GP_OK) {
    scope cleanup_settings{[&]{ gp_widget_free(settings); } };
    auto cameraSetting = CameraSetting::from(settings);
  }
  
  gp_port_info_list_free(portInfoList);
  gp_abilities_list_free(abilities_list);
  
}

void GPhotoCamera::disconnect()
{
  gp_camera_exit(d->camera, d->context);
}

void GPhotoCamera::shoot()
{
  bool delete_file = true;
  CameraTempFile camera_file;
  CameraFilePath camera_remote_file;
  auto fixed_filename = [](const std::string &filename) { return boost::replace_all_copy(filename, "*", ""); };
  QImage image;
  gp_api{{
    sequence_run( [&]{ return gp_camera_capture(d->camera, GP_CAPTURE_IMAGE, &camera_remote_file, d->context);} ),
    sequence_run( [&]{ return gp_camera_file_get(d->camera, camera_remote_file.folder, fixed_filename(camera_remote_file.name).c_str(), GP_FILE_TYPE_NORMAL, camera_file, d->context); } ),
    sequence_run( [&]{ return camera_file.save();} ),
  }}.run_last([&]{
    if(delete_file) {
      int retry = 3;
      for(int i=1; i<=3; i++) {
	int result = gp_camera_file_delete(d->camera, camera_remote_file.folder, fixed_filename(camera_remote_file.name).c_str(), d->context);
	if(result == GP_OK)
	  break;
	if(i<retry)
	  QThread::currentThread()->msleep(500);
	else
	  emit error(this, QString("Error removing image on camera: %1/%2")
	    .arg(camera_remote_file.folder)
	    .arg(QString::fromStdString(fixed_filename(camera_remote_file.name))));
      }
    }
    qDebug() << "shoot completed: camera file " << camera_file.path();
    if(image.load(camera_file)) {
      emit preview(image);
      return;
    }
    qDebug() << "Unable to load image; trying to convert it using GraphicsMagick.";
    Magick::Image m_image;
    m_image.read(camera_file.path().toStdString());
    Magick::Blob blob;
    m_image.write(&blob, "PNG");
    QByteArray data(static_cast<int>(blob.length()), 0);
    std::copy(reinterpret_cast<const char*>(blob.data()), reinterpret_cast<const char*>(blob.data()) + blob.length(), begin(data));
    if(image.loadFromData(data)) {
      emit message(this, "image captured correctly");
      emit preview(image);
      return;
    }
    qDebug() << "Error loading image.";
    emit error(this, "Error loading image");
  }).on_error([=](int errorCode, const std::string &label) {
    const char *errorMessage = gp_result_as_string(errorCode);
    qDebug() << "on " << QString::fromStdString(label) << ": " << errorMessage << "(" << errorCode << ")";
    emit error(this, QString::fromLocal8Bit(errorMessage));
  });
}

QString GPhotoCamera::about() const
{
  return d->about;
}

QString GPhotoCamera::model() const
{
  return d->model;
}

QString GPhotoCamera::summary() const
{
  return d->summary;
}


GPhotoCamera::~GPhotoCamera()
{
  disconnect(); // TODO: check if connected
  gp_camera_free(d->camera);
}




CameraTempFile::CameraTempFile()
{
  int r = gp_file_new(&camera_file);
  qDebug() << __PRETTY_FUNCTION__ << ": gp_file_new=" << r;
  temp_file.open();
  temp_file.close();
  temp_file.setAutoRemove(false);
}

int CameraTempFile::save()
{
  qDebug() << __PRETTY_FUNCTION__;
  return gp_file_save(camera_file, temp_file.fileName().toLocal8Bit());
}

CameraTempFile::~CameraTempFile()
{
  gp_file_free(camera_file);
}

QString CameraTempFile::mimeType() const
{
  int r = gp_file_detect_mime_type(camera_file);
  qDebug() << __PRETTY_FUNCTION__ << ": gp_file_detect_mime_type=" << r;
  const char *mime;
  r = gp_file_get_mime_type(camera_file, &mime);
  qDebug() << __PRETTY_FUNCTION__ << ": gp_file_get_mime_type=" << r;
  return QString(mime);
}
