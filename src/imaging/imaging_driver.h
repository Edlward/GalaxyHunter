#ifndef IMAGER_DRIVER_H
#define IMAGER_DRIVER_H

#include <QDebug>
#include <QObject>
#include <QImage>
#include <QStringList>
#include <memory>

class ShooterSettings;
class ShooterSettings;
class LogMessage;

class QImage;
class ImagingDriver;
typedef std::shared_ptr<ImagingDriver> ImagingDriverPtr;

class Image {
public:
  typedef std::shared_ptr<Image> ptr;
  virtual operator QImage() const = 0;
  void save(const QString &directory, const QString &filename = {});
protected:
  virtual void save_to(const QString& path) = 0;
  virtual QString originalFileName() = 0;
};
Q_DECLARE_METATYPE(Image::ptr)

class Imager : public QObject {
  Q_OBJECT
public:
  struct Settings {
  public:
    struct ComboSetting {
      QString current;
      QStringList available;
      operator bool() const;
      bool operator==(const ComboSetting &other) const;
    };
    ComboSetting shutterSpeed;
    ComboSetting imageFormat;
    ComboSetting iso;
    bool manualExposure;
    qulonglong manualExposureSeconds;
    QString serialShootPort;
    bool operator==(const Settings &other) const;
    operator bool() const;
    QString toString(bool compact = false) const;
  };
  struct Info {
    QString summary;
    QString model;
    QString about;
  };
  
  virtual Info info() const = 0;
  virtual Settings settings() const = 0;
      
public slots:
  virtual void connect() = 0;
  virtual void disconnect() = 0;
  virtual Image::ptr shoot(const Settings &settings) const = 0;
  
signals:
  void connected();
  void disconnected();
  void message(Imager *, const QString &);
  void error(Imager *,const QString &);
  void exposure_remaining(int seconds) const;
};

typedef std::shared_ptr<Imager> ImagerPtr;

class ImagingDriver : public QObject {
    Q_OBJECT
public:
    ImagingDriver(QObject *parent = 0);
    std::vector<ImagerPtr> imagers() const { return _imagers; }
protected:
  std::vector<ImagerPtr> _imagers;
  virtual void scan_imagers() = 0;
public slots:
  void scan();
  void camera_message(Imager* camera, const QString& message);
  void camera_error(Imager *camera, const QString &message);
signals:
  void imager_message(const LogMessage&);
  void camera_connected();
  void scan_finished();
};

class ImagingDrivers : public ImagingDriver {
  Q_OBJECT
public:
  ImagingDrivers(ShooterSettings& shooterSettings, QObject* parent = 0);

protected:
  virtual void scan_imagers();
private:
  QList<ImagingDriverPtr> imagingDrivers;
};

QDebug operator<<(QDebug dbg, const Imager::Settings &settings);
#endif // IMAGER_DRIVER_H
