#ifndef GULINUX_UTILS_QT
#define GULINUX_UTILS_QT

#include <QDebug>
#include <QStringList>

QDebug &operator<<(QDebug &d, const std::string &s) {
  d << QString::fromStdString(s);
  return d;
}

template<class T>
QStringList qstringlist(const T &c) {
  QStringList r;
  std::transform(std::begin(c), std::end(c), std::back_inserter(r), [](const std::string &s) { return QString::fromStdString(s); });
  return r;
}

#endif
