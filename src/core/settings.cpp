#include "settings.h"
#include <QSettings>

using namespace std;

class Settings::Private {
public:
    Private(Settings *q);
    QSettings qsettings;
private:
    Settings *q;
};

Settings::Private::Private(Settings* q) : qsettings{"GuLinux", "GalaxyHunter"}, q(q)
{
}

Settings::Settings()
    : dptr(this)
{
}

Settings::~Settings()
{
}

void Settings::setValue(const QString& key, const QVariant& value)
{
  d->qsettings.setValue(key, value);
}

QVariant Settings::value(const QString& key, const QVariant default_value) const
{
  return d->qsettings.value(key, default_value);
}


Settings::ptr Settings::instance()
{
  static ptr _instance = make_shared<Settings>();
  return _instance;
}

Settings::operator QSettings&()
{
  return d->qsettings;
}

