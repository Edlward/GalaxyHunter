#ifndef SETTINGS_H
#define SETTINGS_H

#include "dptr.h"
#include <QVariant>
#include <QString>

class QSettings;
class Settings
{
public:
    Settings();
    ~Settings();
    QVariant value(const QString &key, const QVariant default_value = {}) const;
    void setValue(const QString &key, const QVariant &value);
    typedef std::shared_ptr<Settings> ptr;
    static ptr instance();
    operator QSettings &();
private:
    D_PTR
};

#endif // SETTINGS_H
