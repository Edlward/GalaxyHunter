#include "gphoto_camera_p.h"
#include "serialshoot.h"
#include <file2image.h>
#include <iostream>
#include <QTimer>
#include <QDir>
#include "commons/shootersettings.h"

#include "utils/qt.h"
#include <sstream>
#include "Qt/strings.h"
#include <QFuture>

GPhotoCamera::Private::Private ( const shared_ptr< GPhotoCameraInformation >& info, ShooterSettings& shooterSettings, GPhotoCamera* q )
  : port(info->port), context(info->context), mutex(info->mutex), shooterSettings{shooterSettings}, q(q)
{
  this->info.model = QString::fromStdString(info->name);
}


void GPhotoCamera::Private::gphoto_error ( int error_code, const QString& file, int line )
{
  const char *errorMessage = gp_result_as_string(error_code);
  qDebug() << "gphoto error on " << file << ":" << line << ": " << errorMessage;
  emit q->error(q, QString(errorMessage));
}



GPhotoCamera::Private::GPhotoComboSetting::GPhotoComboSetting ( GPhotoCamera::Private* d, const QString& settingName )
  : d{d}, settingName{settingName}
{
  load();
}


void GPhotoCamera::Private::GPhotoComboSetting::load()
{
  CameraWidget *settings;
  CameraWidget *widget;
  char *value;
  
  qDebug() << __PRETTY_FUNCTION__ << ": Loading setting:" << settingName;
  comboSetting.available.clear();
  
  int result = gp_camera_get_config(d->camera, &settings, d->context);
  GPHOTO_CHECK_ERROR(result, d)
  result = gp_widget_get_child_by_name(settings, settingName.toLatin1(), &widget);
  GPHOTO_CHECK_ERROR(result, d)
  result = gp_widget_get_value(widget, &value);
  GPHOTO_CHECK_ERROR(result, d)
  comboSetting.current = QString(value);
  int choices = gp_widget_count_choices(widget);
  for(int i=0; i<choices; i++) {
    const char *choice;
    if(gp_widget_get_choice(widget, i, &choice) == GP_OK)
      comboSetting.available.push_back(QString(choice));
  }
  gp_widget_free(settings);
  qDebug() << __PRETTY_FUNCTION__ << ": Setting" << settingName << "value:" << comboSetting.current;
}



void GPhotoCamera::Private::GPhotoComboSetting::save ( const Imager::Settings::ComboSetting& imagerSettings )
{
  if( imagerSettings == this->comboSetting)
    return;
  
  CameraWidget *settings;
  CameraWidget *widget;
  int error_code;
  qDebug() << "setting widget " << settingName << " value to " << imagerSettings.current;
  GPHOTO_RUN( gp_camera_get_config(d->camera, &settings, d->context), d);
  GPHOTO_RUN(gp_widget_get_child_by_name(settings, settingName.toLatin1(), &widget), d)
  GPHOTO_RUN(gp_widget_set_value(widget, imagerSettings.current.toStdString().c_str() ), d)
  GPHOTO_RUN(gp_widget_set_changed(widget, true ), d)
  GPHOTO_RUN( gp_camera_set_config(d->camera, settings, d->context), d);
  gp_widget_free(settings);
  load();
}


GPhotoCamera::GPhotoCamera(const shared_ptr< GPhotoCameraInformation > &gphotoCameraInformation, ShooterSettings &shooterSettings)
  : dptr(gphotoCameraInformation, shooterSettings, this)
{
  int error_code;
  GPHOTO_RUN(gp_camera_new(&d->camera), d)
}


#define IMAGE_FORMAT_SETTING "main/settings/imageformat"
#define ISO_SETTING "main/settings/iso"
#define SHUTTER_SPEED_SETTING "main/settings/shutterspeed"


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
    sequence_run( [&]{ model = gp_abilities_list_lookup_model(abilities_list, d->info.model.toLocal8Bit()); return model; } ),
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
    d->GPHOTO_RETURN_ERROR(errorCode);
  }).run_last([&]{
    d->info.summary = QString(camera_summary.text);
    d->info.about = QString(camera_about.text);
    emit connected();    
  });  
  // TODO d->reloadSettings();
  gp_port_info_list_free(portInfoList);
  gp_abilities_list_free(abilities_list);
  d->imagerSettings.imageFormat = Private::GPhotoComboSetting(d.get(), "imageformat");
  d->imagerSettings.iso = Private::GPhotoComboSetting(d.get(), "iso");
  d->imagerSettings.shutterSpeed = Private::GPhotoComboSetting(d.get(), "shutterspeed");
//   d->imagerSettings.manualExposure = d->shooterSettings.
}

Imager::Settings GPhotoCamera::settings() const
{
    return d->imagerSettings;
}



void GPhotoCamera::disconnect()
{
  gp_camera_exit(d->camera, d->context);
}

Image::ptr GPhotoCamera::shoot(const Imager::Settings &settings) const
{
  qDebug() << "setting camera: " << settings;
  Private::GPhotoComboSetting(d.get(), "imageformat").save(settings.imageFormat);
  Private::GPhotoComboSetting(d.get(), "shutterspeed").save(settings.shutterSpeed);
  Private::GPhotoComboSetting(d.get(), "iso").save(settings.iso);
  if(settings.manualExposure ) {
    return d->shootTethered(settings);
  }
  return d->shootPreset();
}


QString GPhotoCamera::Private::fixedFilename(QString fileName) const
{
  return fileName.replace("*", "");
}


Image::ptr GPhotoCamera::Private::shootPreset(  )
{
  auto camera_file = make_shared<CameraTempFile>(q);
  CameraFilePath camera_remote_file;
  QImage image;
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
}



Image::ptr GPhotoCamera::Private::shootTethered( const Imager::Settings& settings )
{
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
}


CameraTempFile::operator QImage() const {
  try {
    QImage image;
    QFileInfo fileInfo(originalName);
    File2Image file2image(image);
    file2image.load(*this, fileInfo.suffix().toLower());
    return image;
  } catch(std::exception &e) {
      imager->error(imager, QString("Error converting image: %1").arg(e.what()));
      return QImage();
  }
}

void CameraTempFile::save_to(const QString& path){
  QFile file( temp_file.fileName() );
  if(file.copy(path))
    imager->message(imager, "Saved image to %1"_q % path);
  else
    imager->error(imager, "Error saving temporary image %1 to %2"_q % temp_file.fileName() % path);
}

QString CameraTempFile::originalFileName()
{
  return QFileInfo(originalName).fileName();
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
  disconnect(); // TODO: check if connected
  gp_camera_free(d->camera);
}




CameraTempFile::CameraTempFile(GPhotoCamera *imager) : imager{imager}
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

Imager::Info GPhotoCamera::info() const
{
  return d->info;
}

