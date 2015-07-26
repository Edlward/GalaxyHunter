#ifndef GULINUX_UTILS_QT
#define GULINUX_UTILS_QT

#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QtConcurrent>

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

template<typename T> QFutureWatcher<T> *qt_async(std::function<T()> runAsync, std::function<void(T)> runOnFinish) {
  auto futureWatcher = new QFutureWatcher<T>();
  QObject::connect(futureWatcher, &QFutureWatcher<T>::finished, [=]{
    runOnFinish(futureWatcher->result());
    futureWatcher->deleteLater();
  });
  futureWatcher->setFuture(QtConcurrent::run(runAsync));
}



#include <QString>
#include <QDebug>
#ifdef IN_IDE_PARSER
#define _q + QString()
#else
inline QString operator ""_q(const char *s, std::size_t) { return QString{s}; }
#endif

template<typename T>
QString operator%(const QString &other, const T &t) {
  return other.arg(t);
}


template<>
inline QString operator%(const QString &first, const std::string &second) {
  return first.arg(QString::fromStdString(second));
}


#endif
