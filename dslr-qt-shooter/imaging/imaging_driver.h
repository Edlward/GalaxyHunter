#ifndef IMAGER_DRIVER_H
#define IMAGER_DRIVER_H

#include <QObject>
#include <QStringList>
#include <memory>


class QImage;

class Imager : public QObject {
  Q_OBJECT
public:
  class Settings {
  public:
    struct ComboSetting {
      QString current;
      QStringList available;
      operator bool() const { return !current.isEmpty() && !available.empty(); }
    };
    virtual ~Settings() {};
    virtual ComboSetting shutterSpeed() const = 0;
    virtual ComboSetting imageFormat() const = 0;
    virtual ComboSetting iso() const = 0;
    virtual void setShutterSpeed(const QString &) = 0;
    virtual void setImageFormat(const QString &) = 0;
    virtual void setISO(const QString &) = 0;
    virtual void setManualExposure(uint64_t seconds) = 0;
    virtual uint64_t manualExposure() const = 0;
    virtual void setSerialShootPort(const std::string serialShootPort) = 0;
    virtual std::string serialShootPort() const = 0;
    virtual void reload() = 0;
    virtual void apply() = 0;
  };
  
  virtual QString summary() const = 0; // TODO: documentation
  virtual QString model() const = 0;  // TODO: documentation
  virtual QString about() const = 0; // TODO: documentation
  std::shared_ptr<Settings> settings() const { return _settings; }
        
    
public slots:
  virtual void connect() = 0;
  virtual void disconnect() = 0;
  virtual void shoot() = 0;
  virtual void setDeletePicturesOnCamera(bool del) { deletePicturesOnCamera = del; }
  virtual void setOutputDirectory(const QString &directory) = 0;
  void reloadSettings() { _settings->reload(); emit settingsReady(); }
  void applySettings() { _settings->apply(); emit settingsSaved(); }
  
signals:
  void connected();
  void disconnected();
  void message(Imager *, const QString &);
  void error(Imager *,const QString &);
  void preview(const QImage &);
  void settingsReady();
  void settingsSaved();
protected:
  bool deletePicturesOnCamera = false;
  std::shared_ptr<Settings> _settings;
};

class ImagingDriver : public QObject {
    Q_OBJECT
public:
    ImagingDriver(QObject *parent = 0);
    virtual std::vector<std::shared_ptr<Imager>> imagers() const { return _imagers; }
    static ImagingDriver *imagingDriver(QObject *parent = 0);
protected:
  std::vector<std::shared_ptr<Imager>> _imagers;
  virtual void scan_imagers() = 0;
public slots:
  void scan();
  void camera_message(Imager* camera, const QString& message);
  void camera_error(Imager *camera, const QString &message);
signals:
  void imager_message(const QString &);
  void imager_error(const QString &);
  void camera_connected();
  void scan_finished();
};

#endif // IMAGER_DRIVER_H
