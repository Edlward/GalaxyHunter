#include "imager.h"
#include <commons/logmessage.h>
#include "commons/shootersettings.h"
#include "utils/qt.h"
#include <QDir>
#include "Qt/strings.h"

using namespace std;
using namespace std::placeholders;
bool Imager::Settings::ComboSetting::operator== ( const Imager::Settings::ComboSetting& other ) const
{
    return other.available == available && other.current == current;
}

Imager::Settings::ComboSetting::operator bool() const
{
    return !available.empty();
}

Imager::Settings::operator bool() const
{
    if(iso.available.empty() && shutterSpeed.available.empty() && imageFormat.available.empty())
        return false;
    if(shutterSpeed.current.isEmpty() && manualExposureSeconds == 0)
        return false;
    return true;
}


bool Imager::Settings::operator== ( const Imager::Settings& other ) const
{
    return
        shutterSpeed == other.shutterSpeed &&
        serialShootPort == other.serialShootPort &&
        iso == other.iso &&
        manualExposure == other.manualExposure &&
        imageFormat == other.imageFormat;
    ;
}


QString Imager::Settings::toString(bool compact) const
{
    QStringList values;
    values << imageFormat.current
           << "ISO: %1"_q % iso.current
           << "shutter: %1"_q % shutterSpeed.current
           << ( manualExposure ? QTime {0,0,0} .addSecs(manualExposureSeconds ).toString(): "")
           << (!manualExposure || serialShootPort.isEmpty() ? "" : "trigger: %1"_q % serialShootPort);
    values.removeAll({});
    return values.join(", ");
}



QDebug operator<<(QDebug dbg, const Imager::Settings &settings) {
    dbg.nospace().noquote() << "Imager Settings: { "
            << "imageFormat=" << settings.imageFormat << ", "
            << "iso=" << settings.iso << ", "
            << "shutterSpeed=" << settings.shutterSpeed << ", "
            << "manualExposure=" << settings.manualExposure << ", "
            << "serialPort=" << settings.serialShootPort << " }"
            ;
    return dbg.space().quote();
}


void Image::save(const QString& directory, const QString& filename)
{
    auto path = "%1%2%3"_q % directory % QDir::separator() % (filename.isEmpty() ? originalFileName() : filename);
    save_to(path);
}

QDebug operator<<(QDebug dbg, const Imager::Settings::ComboSetting combo) {
    QStringList avail;
    transform(begin(combo.available), end(combo.available), back_inserter(avail), [=](const QString &s) {
        return s==combo.current ? "*%1"_q % s : s;
    });
    auto debug = dbg.noquote().nospace() << "{ " << avail.join(", ") << " }";
    return debug.space().quote();
}


class RegisterImagePtrMetaType {
public:
    RegisterImagePtrMetaType() {
        qRegisterMetaType<Image::ptr>();
    }
};

RegisterImagePtrMetaType __registerImagePtrMetaType;


