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



void GPhotoCamera::Settings::setImageFormat(const QString &v)
{
  changed = true;
  _imageFormat.current = v;
}

void GPhotoCamera::Settings::setISO(const QString &v)
{
  changed = true;
  _iso.current = v;
}

void GPhotoCamera::Settings::setManualExposure(uint64_t seconds)
{
  q->d->manualExposure = seconds;
}


uint64_t GPhotoCamera::Settings::manualExposure() const
{
  return q->d->manualExposure;
}


void GPhotoCamera::Settings::setShutterSpeed(const QString &v)
{
  changed = true;
  _shutterSpeed.current = v;
}

GPhotoCamera::Settings::Settings(GPContext* context, Camera* camera, GPhotoCamera* q)
  : context(context), camera(camera), q(q)
{
  auto combo_widget_to_combosetting = [=] (CameraWidget *widget, ComboSetting &setting){
    void *value;
    int ret = gp_widget_get_value(widget, value);
    if(ret >= GP_OK) {
      setting.current = QString::fromStdString({reinterpret_cast<char*>(value)});
      for(int i=0; i<gp_widget_count_choices(widget); i++) {
	const char *choice;
	if(gp_widget_get_choice(widget, i, &choice) >= GP_OK)
	  setting.available.push_back(QString::fromStdString({choice}));
      }
    }
    return ret;
  };
  gp_api{{
    sequence_run([&] { return gp_camera_get_config(camera, &settings, context); }),
    sequence_run([&] { return gp_widget_get_child_by_name(settings, "imageformat", &imageFormatWidget); }),
    sequence_run([&] { return gp_widget_get_child_by_name(settings, "iso", &isoWidget); }),
    sequence_run([&] { return gp_widget_get_child_by_name(settings, "shutterspeed", &shutterSpeedWidget); }),
    sequence_run([&] { return combo_widget_to_combosetting(imageFormatWidget, _imageFormat); }),
    sequence_run([&] { return combo_widget_to_combosetting(isoWidget, _iso); }),
    sequence_run([&] { return combo_widget_to_combosetting(shutterSpeedWidget, _shutterSpeed); }),
  }};
}

GPhotoCamera::Settings::~Settings()
{
  if(changed)
    gp_api {{
      sequence_run([&]{  return gp_widget_set_value(imageFormatWidget, _imageFormat.current.data()); }),
      sequence_run([&]{  return gp_widget_set_value(shutterSpeedWidget, _shutterSpeed.current.data()); }),
      sequence_run([&]{  return gp_widget_set_value(isoWidget, _iso.current.data()); }),
    }};
  gp_widget_free(settings);
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


#define IMAGE_FORMAT_SETTING "main/settings/imageformat"
#define ISO_SETTING "main/settings/iso"
#define SHUTTER_SPEED_SETTING "main/settings/shutterspeed"



void GPhotoCamera::querySettings()
{
  emit settings(make_shared<Settings>(d->context, d->camera, this));
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
  // TODO d->reloadSettings();
  gp_port_info_list_free(portInfoList);
  gp_abilities_list_free(abilities_list);
  
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
    CameraFilePath *newfile;
    
    gp_api{{
      sequence_run([&]{ return gp_camera_wait_for_event(camera, 10000, &event, &data, context); }),
      sequence_run([&]{ return event == GP_EVENT_FILE_ADDED ? GP_OK : -1; }),
      sequence_run([&]{ 
        newfile = reinterpret_cast<CameraFilePath*>(data);
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
	if(file.copy(destination))
	  q->message(q, QString("Saved image to %1").arg(destination));
	else
	  q->error(q, QString("Error saving image to %1").arg(destination));
      }
      deletePicturesOnCamera(*newfile);
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
  temp_file.setAutoRemove(true);
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
/*
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
}*/
