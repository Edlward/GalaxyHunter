#include "gphoto_camera_p.h"
#include "serialshoot.h"
#include <file2image.h>
#include "imager.h"
#include <iostream>
#include <QTimer>
#include <QDir>
#include "commons/shootersettings.h"

#include "utils/qt.h"
#include <sstream>
#include "Qt/strings.h"
#include <QFuture>
using namespace std;
using namespace std::placeholders;

GPhotoCamera::Private::Private ( const GPhotoCPP::Driver::CameraFactory::ptr& info, ShooterSettings& shooterSettings, GPhotoCamera* q )
  : factory{info}, shooterSettings{shooterSettings}, q(q)
{
}


GPhotoCamera::GPhotoCamera(const GPhotoCPP::Driver::CameraFactory::ptr &gphotoCameraInformation, ShooterSettings &shooterSettings)
  : dptr(gphotoCameraInformation, shooterSettings, this)
{
}



void GPhotoCamera::connect()
{
  d->camera = *d->factory;
  if(d->camera) {
    // TODO: add logger
    emit connected();
    d->info.model = QString::fromStdString(d->factory->name());
    d->info.summary = QString::fromStdString(d->camera->summary());
    
    function<QString(string)> transform_f = bind(&QString::fromStdString, _1);
    d->init_combo_settings(d->camera->settings().iso(), d->camera->settings().iso_choices(), d->imagerSettings.iso, transform_f);
    d->init_combo_settings(d->camera->settings().format(), d->camera->settings().format_choices(), d->imagerSettings.imageFormat, transform_f);
    function<QString(GPhotoCPP::Exposure::Value)> transform_value = [](const GPhotoCPP::Exposure::Value &v){ return QString::fromStdString(v.text); };
    d->init_combo_settings(d->camera->settings().exposure()->value(), d->camera->settings().exposure()->values(), d->imagerSettings.shutterSpeed, transform_value);
  }
}

Imager::Settings GPhotoCamera::settings() const
{
    return d->imagerSettings;
}



void GPhotoCamera::disconnect()
{
  d->camera.reset();
  emit disconnected();
}

Image::ptr GPhotoCamera::shoot(const Imager::Settings &settings) const
{
  qDebug() << "setting camera: " << settings;
  d->camera->settings().set_iso(settings.iso.current.toStdString());
  d->camera->settings().set_format(settings.imageFormat.current.toStdString());
  GPhotoCPP::milliseconds exposure;
  if(settings.manualExposure) {
    exposure = GPhotoCPP::seconds{settings.manualExposureSeconds};
  } else {
    auto exposures = d->camera->settings().exposure()->values();
    auto exposure_v = find_if(begin(exposures), end(exposures), [&](const GPhotoCPP::Exposure::Value &v){ return v.text == settings.shutterSpeed.current.toStdString(); });
    exposure = (*exposure_v).duration();
  }
  auto shot = d->camera->control().shoot(exposure, settings.mirrorLock);
  shot->camera_file().wait();
  return make_shared<CameraTempFile>(shot->camera_file().get());
//   Private::GPhotoComboSetting(d.get(), "imageformat").save(settings.imageFormat);
//   Private::GPhotoComboSetting(d.get(), "shutterspeed").save(settings.shutterSpeed);
//   Private::GPhotoComboSetting(d.get(), "iso").save(settings.iso);
//   if(settings.manualExposure ) {
//     return d->shootTethered(settings);
//   }
//   return d->shootPreset();
}


QString GPhotoCamera::Private::fixedFilename(QString fileName) const
{
  return fileName.replace("*", "");
}


Image::ptr GPhotoCamera::Private::shootPreset(  )
{
  /*
  auto camera_file = make_shared<CameraTempFile>(q);
  CameraFilePath camera_remote_file;
  QImage image;duration
  gp_api{{
    sequence_run( [&]{ return gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_remote_file, context);} ),
    sequence_run( [&]{ return gp_camera_file_get(camera, camera_remote_file.folder, fixedFilename(camera_remote_file.name).toLatin1(), GP_FILE_TYPE_NORMAL, *camera_file, context); } ),
    sequence_run( [&]{ return camera_file->save();} ),
  }, make_shared<QMutexLocker>(&mutex)}.run_last([&]{
    camera_file->originalName = fixedFilename(camera_remote_file.name);
//     deletePicturesOnCamera(camera_remote_file); TODO: add again?
  }).on_error([=](int errorCode, const std::string &label) {
    GPHOTO_RETURN_ERROR(errorCode);
  });
  return camera_file;
  */
}



Image::ptr GPhotoCamera::Private::shootTethered( const Imager::Settings& settings )
{
  /*
  auto shoot_future = QtConcurrent::run([=]{
    auto shoot = make_shared<SerialShoot>(settings.serialShootPort.toStdString());
    if(settings.mirrorLock) {
      shoot.reset();
      QThread::currentThread()->sleep(2);
      shoot = make_shared<SerialShoot>(settings.serialShootPort.toStdString());
    }
    QElapsedTimer elapsed;
    elapsed.start();
    int elapsed_secs = 0;
    while(elapsed.elapsed() < settings.manualExposureSeconds * 1000) {
      if(elapsed.elapsed()/1000 > elapsed_secs) {
	elapsed_secs = elapsed.elapsed()/1000;
	qDebug() << "elapsed=" << elapsed_secs;
	q->exposure_remaining(settings.manualExposureSeconds-elapsed_secs);
      }
      q->exposure_remaining(0);
    }
  });

    CameraEventType event;
    void *data;
    auto camera_file = make_shared<CameraTempFile>(q);
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
      result = gp_camera_wait_for_event(camera, (settings.manualExposureSeconds*2 + 20)*1000, &event, &data, context);
      qDebug() << "result=" << result << ", event=" << eventTypes[event];
      if(result == GP_OK && event == GP_EVENT_FILE_ADDED)
	break;
    }
    shoot_future.waitForFinished();
    if(event != GP_EVENT_FILE_ADDED)
    {
      GPHOTO_RETURN_ERROR(result, {})
    }
    CameraFilePath *newfile  = reinterpret_cast<CameraFilePath*>(data);
    QString filename = fixedFilename(newfile->name);
    if( result = gp_camera_file_get(camera, newfile->folder, filename.toLatin1(), GP_FILE_TYPE_NORMAL, *camera_file, context) != GP_OK) {
      GPHOTO_RETURN_ERROR(result, {})
    }
    if( result = camera_file->save() != GP_OK) {
      GPHOTO_RETURN_ERROR(result, {})
    }

    camera_file->originalName = filename;
    qDebug() << "Output directory: " << shooterSettings.saveImageDirectory();
//     deletePicturesOnCamera(*newfile);TODO: add again?
    return camera_file;
    */
}


CameraTempFile::operator QImage() const {
  try {
    QImage image;
    QFileInfo fileInfo(originalFileName());
    File2Image file2image(image);
    file2image.load(temp_file.fileName(), fileInfo.suffix().toLower());
    return image;
  } catch(std::exception &e) {
    // TODO: error report
//       imager->error(imager, QString("Error converting image: %1").arg(e.what()));
      return QImage();
  }
}

void CameraTempFile::save_to(const QString& path){
  QFile file( temp_file.fileName() );
  file.copy(path);
  // TODO: error log
//   if(file.copy(path))
//     imager->message(imager, "Saved image to %1"_q % path);
//   else
//     imager->error(imager, "Error saving temporary image %1 to %2"_q % temp_file.fileName() % path);
}

QString CameraTempFile::originalFileName() const
{
  return QString::fromStdString( camera_file->file() );
}


/*TODO: add again?
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

*/



GPhotoCamera::~GPhotoCamera()
{
}




CameraTempFile::CameraTempFile(const GPhotoCPP::CameraFilePtr &camera_file) : camera_file(camera_file)
{
  temp_file.open();
  temp_file.close();
  temp_file.setAutoRemove(true);
  camera_file->save(temp_file.fileName().toStdString());
}


CameraTempFile::~CameraTempFile()
{
  qDebug() << __PRETTY_FUNCTION__ ;
}

QString CameraTempFile::mimeType() const
{
//   int r = gp_file_detect_mime_type(camera_file);
//   qDebug() << __PRETTY_FUNCTION__ << ": gp_file_detect_mime_type=" << r;
//   const char *mime;
//   r = gp_file_get_mime_type(camera_file, &mime);
//   qDebug() << __PRETTY_FUNCTION__ << ": gp_file_get_mime_type=" << r;
//   return QString(mime);
  return QString{}; // TODO
}

Imager::Info GPhotoCamera::info() const
{
  return d->info;
}

