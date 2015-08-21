#include "gphoto_camera_p.h"
#include "serialshoot.h"
#include <file2image.h>
#include <iostream>
#include <QTimer>
#include <QDir>

#include "utils/qt.h"
#include <utils/qlambdathread.h>
#include <sstream>


QString gphoto_error(int errorCode)
{
    const char *errorMessage = gp_result_as_string(errorCode);
    return QString(errorMessage);
}


void GPhotoCamera::Settings::setManualExposure(qulonglong seconds)
{
  q->d->manualExposure = seconds;
}


qulonglong GPhotoCamera::Settings::manualExposure() const
{
  return q->d->manualExposure;
}


string GPhotoCamera::Settings::serialShootPort() const
{
  return q->d->serialShootPort;
}

void GPhotoCamera::Settings::setSerialShootPort(const string serialShootPort)
{
  q->d->serialShootPort = serialShootPort;
}


GPhotoCamera::Settings::Settings(GPContext* context, Camera* camera, GPhotoCamera* q, QMutex &mutex)
  : context(context), camera(camera), q(q), mutex(mutex)
{
  qDebug() << __PRETTY_FUNCTION__ << ": Loading settings";
  gp_api{{
    sequence_run([&] { return gp_camera_get_config(camera, &settings, context); }),
    sequence_run([&] { return gp_widget_get_child_by_name(settings, "imageformat", &_imageFormat.widget); }),
    sequence_run([&] { return gp_widget_get_child_by_name(settings, "iso", &_iso.widget); }),
    sequence_run([&] { return gp_widget_get_child_by_name(settings, "shutterspeed", &_shutterSpeed.widget); }),
  }, make_shared<QMutexLocker>(&mutex)}.run_last([=]{
    _imageFormat.load();
    _iso.load();
    _shutterSpeed.load();
  });
}

int GPhotoCamera::Settings::Set::save()
{
  qDebug() << "Saving setting: current=" << setting.current << ", previous: " << _original;
  if(setting.current == _original)
    return GP_OK;
  return  gp_widget_set_value(widget, setting.current.toStdString().c_str());
}

int GPhotoCamera::Settings::Set::load()
{
  setting.available.clear();
  char *value;
  int ret = gp_widget_get_value(widget, &value);
  if(ret >= GP_OK) {
    setting.current = QString(value);
    _original = setting.current;
    int choices = gp_widget_count_choices(widget);
    for(int i=0; i<choices; i++) {
      const char *choice;
      if(gp_widget_get_choice(widget, i, &choice) == GP_OK)
	setting.available.push_back(QString(choice));
    }
  }
  return ret;
}


GPhotoCamera::Settings::~Settings()
{
  qDebug() << __PRETTY_FUNCTION__;
  vector<Set> sets{_imageFormat, _iso, _shutterSpeed};
  if(any_of(begin(sets), end(sets), [](const Set &s){ return s._original != s.setting.current; })) {
    gp_api {{
      sequence_run([&]{ return _imageFormat.save(); }),
      sequence_run([&]{ return _shutterSpeed.save(); }),
      sequence_run([&]{ return _iso.save(); }),
      sequence_run([&]{ return gp_camera_set_config(camera, settings, context); }),
    }, make_shared<QMutexLocker>(&mutex)}.on_error([=](int errorCode, const std::string &label) {
      qDebug() << gphoto_error(errorCode);
      q->error(q, gphoto_error(errorCode));
    });
  }
  gp_widget_free(settings);
  qDebug() << "done";
}


GPhotoCamera::GPhotoCamera(const shared_ptr< GPhotoCameraInformation > &gphotoCameraInformation)
  : d(new Private{gphotoCameraInformation, this})
{
  gp_api{{
    { [=] { return gp_camera_new(&d->camera); } },
  }, make_shared<QMutexLocker>(&d->mutex)}.on_error([=](int errorCode, const std::string &label) {
    qDebug() << gphoto_error(errorCode);
    emit error(this, gphoto_error(errorCode));
  });
}


#define IMAGE_FORMAT_SETTING "main/settings/imageformat"
#define ISO_SETTING "main/settings/iso"
#define SHUTTER_SPEED_SETTING "main/settings/shutterspeed"

#include <boost/thread.hpp>

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
  }, make_shared<QMutexLocker>(&d->mutex)}.on_error([=](int errorCode, const std::string &label) {
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

shared_ptr< Imager::Settings > GPhotoCamera::settings()
{
    return make_shared<Settings>(d->context, d->camera, this, d->mutex);
}



void GPhotoCamera::disconnect()
{
  gp_camera_exit(d->camera, d->context);
}

QImage GPhotoCamera::shoot() const
{
  if(d->manualExposure > 0) {
    return d->shootTethered();
  }
  return d->shootPreset();
}


string GPhotoCamera::Private::fixedFilename(const string& fileName) const
{
  return boost::replace_all_copy(fileName, "*", "");
}


QImage GPhotoCamera::Private::shootPreset()
{
  CameraTempFile camera_file;
  CameraFilePath camera_remote_file;
  QImage image;
  gp_api{{
    sequence_run( [&]{ return gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_remote_file, context);} ),
    sequence_run( [&]{ return gp_camera_file_get(camera, camera_remote_file.folder, fixedFilename(camera_remote_file.name).c_str(), GP_FILE_TYPE_NORMAL, camera_file, context); } ),
    sequence_run( [&]{ return camera_file.save();} ),
  }, make_shared<QMutexLocker>(&mutex)}.run_last([&]{
    camera_file.originalName = QString::fromStdString(fixedFilename(camera_remote_file.name));
    deletePicturesOnCamera(camera_remote_file);
    image = fileToImage(camera_file);
  }).on_error([=](int errorCode, const std::string &label) {
    qDebug() << "on " << QString::fromStdString(label) << ": " << gphoto_error(errorCode) << "(" << errorCode << ")";
    q->error(q, gphoto_error(errorCode));
  });
  return image;
}


QImage GPhotoCamera::Private::fileToImage(CameraTempFile& cameraTempFile) const
{
  try {
    if(!q->_outputDirectory.isEmpty()) {
      QFile file(cameraTempFile.path());
      auto destination = q->_outputDirectory + QDir::separator() + cameraTempFile.originalName;
      if(file.copy(destination))
	q->message(q, QString("Saved image to %1").arg(destination));
      else
	q->error(q, QString("Error saving image to %1").arg(destination));
    }
    qDebug() << "shoot completed: camera file " << cameraTempFile.path();
    QImage image;
    QFileInfo fileInfo(cameraTempFile.originalName);
    File2Image file2image(image);
    file2image.load(cameraTempFile, fileInfo.suffix().toLower());
    return image;
  } catch(std::exception &e) {
      q->error(q, QString("Error converting image: %1").arg(e.what()));
      return QImage();
  }
}

QImage GPhotoCamera::Private::shootTethered()
{
  boost::thread t([=]{
    auto shoot = make_shared<SerialShoot>(serialShootPort);
    QElapsedTimer elapsed;
    elapsed.start();
    int elapsed_secs = 0;
    while(elapsed.elapsed() < manualExposure * 1000) {
      if(elapsed.elapsed()/1000 > elapsed_secs) {
	elapsed_secs = elapsed.elapsed()/1000;
	q->exposure_remaining(manualExposure-elapsed_secs);
      }
      q->exposure_remaining(0);
    }
  });

    CameraEventType event;
    void *data;
    CameraTempFile camera_file;
    QImage image;
    
    QMap<CameraEventType, QString> eventTypes {
      	{ GP_EVENT_UNKNOWN,	"< unknown and unhandled event " },
	{ GP_EVENT_TIMEOUT,	"< timeout, no arguments " },
	{ GP_EVENT_FILE_ADDED,	"< CameraFilePath* = file path on camfs " },
	{ GP_EVENT_FOLDER_ADDED,	"< CameraFilePath* = folder on camfs " },
	{ GP_EVENT_CAPTURE_COMPLETE,	"< last capture is complete " },
    };
    int result;
    for(int i=0; i<3; i++) {
      result = gp_camera_wait_for_event(camera, (manualExposure*2 + 20)*1000, &event, &data, context);
      qDebug() << "result=" << result << ", event=" << eventTypes[event];
      if(result == GP_OK && event == GP_EVENT_FILE_ADDED)
	break;
    }
    t.join();
    if(event != GP_EVENT_FILE_ADDED)
    {
      q->error(q, gphoto_error(result));
      return {};
    }
    CameraFilePath *newfile  = reinterpret_cast<CameraFilePath*>(data);
    string filename = fixedFilename(newfile->name);
    if( result = gp_camera_file_get(camera, newfile->folder, filename.c_str(), GP_FILE_TYPE_NORMAL, camera_file, context) != GP_OK) {
      q->error(q, gphoto_error(result));
      return {};
    }
    if( result = camera_file.save() != GP_OK) {
      q->error(q, gphoto_error(result));
      return {};
    }

    camera_file.originalName = QString::fromStdString(filename);
    qDebug() << "Output directory: " << q->_outputDirectory;
    deletePicturesOnCamera(*newfile);
    image = fileToImage(camera_file);
    return image;
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
  qDebug() << __PRETTY_FUNCTION__ ;
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
