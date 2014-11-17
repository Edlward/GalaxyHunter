#include "gphoto_camera_p.h"
#include "serialshoot.h"
#include <iostream>
#include <QTimer>
#include <QDir>

#include "utils/qt.h"

QString gphoto_error(int errorCode)
{
    const char *errorMessage = gp_result_as_string(errorCode);
    return QString(errorMessage);
}


GPhotoCamera::GPhotoCamera(const shared_ptr< GPhotoCameraInformation > &gphotoCameraInformation)
  : d(new Private{gphotoCameraInformation, this})
{
  gp_api{{
    { [=] { return gp_camera_new(&d->camera); } },
  }}.on_error([=](int errorCode, const std::string &label) {
    qDebug() << gphoto_error(errorCode);
    emit error(this, gphoto_error(errorCode));
  });
}

#define enum_pair(Value) {Value, #Value}
ostream &operator<<(ostream &o, const CameraSetting &s) {
  static map<CameraWidgetType, string> types {
  enum_pair(GP_WIDGET_WINDOW), enum_pair(GP_WIDGET_SECTION), enum_pair(GP_WIDGET_TEXT), enum_pair(GP_WIDGET_RANGE),
  enum_pair(GP_WIDGET_TOGGLE), enum_pair(GP_WIDGET_RADIO), enum_pair(GP_WIDGET_MENU), enum_pair(GP_WIDGET_BUTTON), enum_pair(GP_WIDGET_DATE),
  };
  o << s.path() << ": " << ", " << s.label << ", " << s.info << ", " << types[s.type];
  
  if(s.type == GP_WIDGET_TEXT || s.type == GP_WIDGET_RADIO || s.type == GP_WIDGET_MENU)
    o << "; value: " << s.value;
  if(s.type == GP_WIDGET_RADIO || s.type == GP_WIDGET_MENU) {
    string sep = "";
    o << "\nchoices: ";
    for(auto choice: s.choices) {
      o << sep << choice;
      sep = ", ";
    }
  }
  o << endl;
  for(auto sub: s.children)
    o << *sub;
  return o;
}

#define IMAGE_FORMAT_SETTING "main/settings/imageformat"
#define ISO_SETTING "main/settings/iso"
#define SHUTTER_SPEED_SETTING "main/settings/shutterspeed"

Imager::ComboSetting GPhotoCamera::imageFormat() const
{
  auto setting = d->settings->find(IMAGE_FORMAT_SETTING);
  if(!setting) return {};
  return {QString::fromStdString(setting->value), qstringlist(setting->choices)};
}

Imager::ComboSetting GPhotoCamera::iso() const
{
  auto setting = d->settings->find(ISO_SETTING);
  if(!setting) return {};
  return {QString::fromStdString(setting->value), qstringlist(setting->choices)};
}

Imager::ComboSetting GPhotoCamera::shutterSpeed() const
{
  auto setting = d->settings->find(SHUTTER_SPEED_SETTING);
  if(!setting) return {};
  return {QString::fromStdString(setting->value), qstringlist(setting->choices)};
}


void GPhotoCamera::setImageFormat(const QString& imageFormat)
{
    d->setting(IMAGE_FORMAT_SETTING, imageFormat);
}

void GPhotoCamera::setISO(const QString& iso)
{
    d->setting(ISO_SETTING, iso);
}

void GPhotoCamera::setShutterSpeed(const QString& speed)
{
    d->setting(SHUTTER_SPEED_SETTING, speed);
}

shared_ptr< CameraSetting > CameraSetting::find(const string& _path)
{
  if(_path == path())
    return shared_from_this();
  for(auto child: children) {
    auto found = child->find(_path);
    if(found)
      return found;
  }
  return {};
}




void GPhotoCamera::Private::setting(const std::string &path, const QString& value)
{
  auto s = settings->find(path);
  if(!s) return;
  qDebug() << "finding child with path: " << s->path() << ", id: " << s->id << ", label: " << s->label;
  CameraWidget *settings = nullptr;
  CameraWidget *setting;
  
  gp_api{{
    sequence_run( [&]{ return gp_camera_get_config(camera, &settings, context); }),
    sequence_run( [&]{ return gp_widget_get_child_by_name(settings, s->name.c_str(), &setting); }),
    sequence_run( [&]{ return gp_widget_set_value(setting, value.toStdString().c_str()); }),
    sequence_run( [&]{ return gp_camera_set_config(camera, settings, context); }),
  }}.on_error([&](int errorCode, const std::string &label) {
    qDebug() << "Error writing setting " << value << " for widget " << path << " on " << label << ": " << gphoto_error(errorCode);
    q->error(q, gphoto_error(errorCode));
  }).run_last([&]{
    int id = 0;
    gp_widget_get_id(setting, &id);
    qDebug() << "new id: " << id;
  });
  
  gp_widget_free(settings);
  reloadSettings();  
}


string CameraSetting::path() const
{
  auto v = shared_from_this();
  boost::filesystem::path p{v->name};
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
    sequence_run( [&]{ 
      if(setting->type == GP_WIDGET_TEXT || setting->type == GP_WIDGET_RADIO || setting->type == GP_WIDGET_MENU) {
	char *text;
	int ret = gp_widget_get_value(widget, &text);
	setting->value = string{text};
	return ret;
      }
      return GP_OK;
    }),
    sequence_run( [&]{ 
      if(setting->type == GP_WIDGET_RADIO || setting->type == GP_WIDGET_MENU) {
	for(int i=0; i<gp_widget_count_choices(widget); i++) {
	  const char *text;
	  int ret = gp_widget_get_choice(widget, i, &text);
	  if(ret != GP_OK) return ret;
	  setting->choices.push_back({text});
	}
      }
      return GP_OK;
    }),
  }}.on_error([&](int errorCode, const std::string &label) {
    qDebug() << "Error decoding setting on " << label << ": " << gphoto_error(errorCode);
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
    qDebug() << "on " << label << ": " << gphoto_error(errorCode);
    emit error(this, gphoto_error(errorCode));
  }).run_last([&]{
    d->summary = QString(camera_summary.text);
    d->about = QString(camera_about.text);
    emit connected();    
  });  
  d->reloadSettings();
  gp_port_info_list_free(portInfoList);
  gp_abilities_list_free(abilities_list);
  
}

void GPhotoCamera::Private::reloadSettings()
{
  CameraWidget *settings;
  if(gp_camera_get_config(camera, &settings, context) == GP_OK) {
    scope cleanup_settings{[&]{ gp_widget_free(settings); } };
    this->settings = CameraSetting::from(settings);
    cerr << *this->settings << endl;
  }
}

void GPhotoCamera::disconnect()
{
  gp_camera_exit(d->camera, d->context);
}

void GPhotoCamera::shoot()
{
  if(d->manualExposure > 0) {
    d->shootTethered();
    return;
  }
  d->shootPreset();
}


string GPhotoCamera::Private::fixedFilename(const string& fileName) const
{
  return boost::replace_all_copy(fileName, "*", "");
}


void GPhotoCamera::Private::shootPreset()
{
  CameraTempFile camera_file;
  CameraFilePath camera_remote_file;
  QImage image;
  gp_api{{
    sequence_run( [&]{ return gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_remote_file, context);} ),
    sequence_run( [&]{ return gp_camera_file_get(camera, camera_remote_file.folder, fixedFilename(camera_remote_file.name).c_str(), GP_FILE_TYPE_NORMAL, camera_file, context); } ),
    sequence_run( [&]{ return camera_file.save();} ),
  }}.run_last([&]{
    deletePicturesOnCamera(camera_remote_file);
    qDebug() << "shoot completed: camera file " << camera_file.path();
    if(image.load(camera_file)) {
      q->preview(image);
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
      q->message(q, "image captured correctly");
      q->preview(image);
      return;
    }
    qDebug() << "Error loading image.";
    q->error(q, "Error loading image");
  }).on_error([=](int errorCode, const std::string &label) {
    qDebug() << "on " << QString::fromStdString(label) << ": " << gphoto_error(errorCode) << "(" << errorCode << ")";
    q->error(q, gphoto_error(errorCode));
  });
}

void GPhotoCamera::Private::shootTethered()
{
  {
    auto shoot = make_shared<SerialShoot>("/dev/ttyUSB0");
    q->thread()->msleep(manualExposure * 1000);
  }
    CameraEventType event;
    void *data;
    CameraTempFile camera_file;
    string filename;
    
    gp_api{{
      sequence_run([&]{ return gp_camera_wait_for_event(camera, 10000, &event, &data, context); }),
      sequence_run([&]{ return event == GP_EVENT_FILE_ADDED ? GP_OK : -1; }),
      sequence_run([&]{ 
        CameraFilePath *newfile = reinterpret_cast<CameraFilePath*>(data);
	filename = fixedFilename(newfile->name);
        return gp_camera_file_get(camera, newfile->folder, filename.c_str(), GP_FILE_TYPE_NORMAL, camera_file, context);
      }),
      sequence_run( [&]{ return camera_file.save();} ),
    }}.on_error([=](int errorCode, const std::string &label) {
      qDebug() << "on " << QString::fromStdString(label) << ": " << gphoto_error(errorCode) << "(" << errorCode << ")";
      q->error(q, gphoto_error(errorCode));
    }).run_last([&]{
      qDebug() << "Output directory: " << outputDirectory;
      if(!outputDirectory.isEmpty()) {
	QFile file(camera_file.path());
	auto destination = outputDirectory + QDir::separator() + QString::fromStdString(filename);
	bool copied = file.copy(destination);
	qDebug() << "copied to " << destination << ": " << copied;
      }
    });
}

void GPhotoCamera::Private::deletePicturesOnCamera(const CameraFilePath &camera_remote_file)
{
  if(q->deletePicturesOnCamera) {
    int retry = 3;
    for(int i=1; i<=3; i++) {
      int result = gp_camera_file_delete(camera, camera_remote_file.folder, fixedFilename(camera_remote_file.name).c_str(), context);
      if(result == GP_OK)
	break;
      if(i<retry)
	QThread::currentThread()->msleep(500);
      else
	q->error(q, QString("Error removing image on camera: %1/%2")
	  .arg(camera_remote_file.folder)
	  .arg(QString::fromStdString(fixedFilename(camera_remote_file.name))));
    }
  }
}

void GPhotoCamera::setOutputDirectory(const QString& directory)
{
  qDebug() << "OutputDirectory: " << directory;
  d->outputDirectory = directory;
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

uint64_t GPhotoCamera::manualExposure() const
{
    return d->manualExposure;
}

void GPhotoCamera::setManualExposure(uint64_t seconds)
{
    d->manualExposure = seconds;
    if(seconds > 0) {
      setShutterSpeed("Bulb");
    }
}
