#ifndef GULINUX_UTILS_QT
#define GULINUX_UTILS_QT

#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QtConcurrent>



template<typename T> QFutureWatcher<T> *qt_async(std::function<T()> runAsync, std::function<void(T)> runOnFinish) {
  auto futureWatcher = new QFutureWatcher<T>();
  QObject::connect(futureWatcher, &QFutureWatcher<T>::finished, [=]{
    runOnFinish(futureWatcher->result());
    futureWatcher->deleteLater();
  });
  futureWatcher->setFuture(QtConcurrent::run(runAsync));
}


#endif
