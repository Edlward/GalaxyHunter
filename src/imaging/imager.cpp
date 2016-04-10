#define cimg_plugin1 "CImg/plugins/bayer.h"
#include "imager.h"
#include <commons/logmessage.h>
#include "commons/shootersettings.h"
#include "utils/qt.h"
#include <QDir>
#include "Qt/strings.h"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace std::placeholders;

Image::Image(const QString& original_file_name, const vector<uint8_t> &original_data) : original_file_name{original_file_name}, original_data{original_data}
{

}


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

QDebug operator<<(QDebug dbg, const Imager::Settings::ComboSetting& c)
{
  return dbg << "[value=" << c.current << "; available=" << c.available.join(",") << "]";
}



void Image::save(const QString& directory, const QString& filename)
{
    auto path = "%1%2%3"_q % directory % QDir::separator() % (filename.isEmpty() ? original_file_name : filename);
    QFile file{path};
    if(file.open(QIODevice::WriteOnly)) {
      auto written = file.write(reinterpret_cast<const char*>(original_data.data()), original_data.size());
      qDebug() << "written" << written << "bytes out of" << original_data.size() << "bytes to" << path;
    }
}

QImage Image::qimage(bool debayer) const
{
  QImage qimage(image.width(), image.height(), QImage::Format_RGB32);
  auto image = this->image;
  if(debayer && image.spectrum() == 1) {
    image.BayertoRGB();
//     cv::Mat bayer(image.height(), image.width(), CV_16UC1, reinterpret_cast<uint8_t*>(image.data()));
//     cv::Mat debayer;
//     cv::cvtColor(bayer, debayer, CV_BayerGR2RGB);
//     image = cimg_library::CImg<uint16_t>(image.width(), image.height(), image.depth(), 3);
//     
//     cimg_forXY(image, x, y) {
//       auto pixel = debayer.at<cv::Vec3w>(y, x);
//       image(x, y, 0) = pixel[0];
//       image(x, y, 1) = pixel[1];
//       image(x, y, 2) = pixel[2];
//     }
  }
  auto channels = image.normalize(0, 255).get_split('c');
  cimg_forXY(channels[0], x, y) {
    auto r = channels[0](x, y);
    auto g = channels.size() == 3 ? channels[1](x, y) : r;
    auto b = channels.size() == 3 ? channels[2](x, y) : r;
    qimage.setPixel(x, y, qRgb(r, g, b));
  }
  return qimage;
}


QDebug operator<<(QDebug dbg, const Imager::Settings::ComboSetting combo) {
    QStringList avail;
    transform(begin(combo.available), end(combo.available), back_inserter(avail), [=](const QString &s) {
        return s==combo.current ? "*%1"_q % s : s;
    });
    auto debug = dbg.noquote().nospace() << "{ " << avail.join(", ") << " }";
    return debug.space().quote();
}

cimg_library::CImg< uint64_t > Image::histogram(uint32_t bins) const
{
  return image.get_histogram(bins);
}


class RegisterImagePtrMetaType {
public:
    RegisterImagePtrMetaType() {
        qRegisterMetaType<Image::ptr>();
    }
};

RegisterImagePtrMetaType __registerImagePtrMetaType;


