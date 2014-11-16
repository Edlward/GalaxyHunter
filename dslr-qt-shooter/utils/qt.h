#ifndef GULINUX_UTILS_QT
#define GULINUX_UTILS_QT

#include <QDebug>
#include <QStringList>
#include <QTimer>

inline QDebug &operator<<(QDebug &d, const std::string &s) {
  d << QString::fromStdString(s);
  return d;
}

inline std::ostream &operator<<(std::ostream &o, const QString &s) {
  o << s.toStdString();
  return o;
}

template<class T>
inline QStringList qstringlist(const T &c) {
  QStringList r;
  std::transform(std::begin(c), std::end(c), std::back_inserter(r), [](const std::string &s) { return QString::fromStdString(s); });
  return r;
}

inline void timedLambda(int msec, std::function<void()> f, QObject *context) {
  QTimer *timer = new QTimer(context);
  timer->setSingleShot(true);
  QObject::connect(timer, &QTimer::timeout, context, [=] {f(); delete timer; });
  timer->start(msec);
}


#endif
