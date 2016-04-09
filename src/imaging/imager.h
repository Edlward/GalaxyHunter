#ifndef GALAXYHUNTER_IMAGER_H
#define GALAXYHUNTER_IMAGER_H

#include "imaging_driver.h"


class Image {
public:
  typedef std::shared_ptr<Image> ptr;
  virtual operator QImage() const = 0;
  void save(const QString &directory, const QString &filename = {});
protected:
  virtual void save_to(const QString& path) = 0;
  virtual QString originalFileName() const = 0;
};
Q_DECLARE_METATYPE(Image::ptr)


class Imager : public QObject {
  Q_OBJECT
public:
  struct Settings;
  struct Info;
  
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

struct Imager::Info {
  QString summary;
  QString model;
  QString about;
};

struct Imager::Settings {
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
  bool mirrorLock;
  bool operator==(const Settings &other) const;
  operator bool() const;
  QString toString(bool compact = false) const;
};

QDebug operator<<(QDebug dbg, const Imager::Settings &settings);
QDebug operator<<(QDebug dbg, const Imager::Settings::ComboSetting &c);


#endif