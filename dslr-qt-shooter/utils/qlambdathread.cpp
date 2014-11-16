#include "qlambdathread.h"

QLambdaThread::QLambdaThread(QThread* thread, std::function<void()> f) : QObject(), f(f) {
    moveToThread(thread);
    QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
}

void QLambdaThread::start()
{
 f();
 deleteLater();
}
