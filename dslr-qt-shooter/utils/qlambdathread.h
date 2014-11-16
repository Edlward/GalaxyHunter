#ifndef QT_UTILS_QLAMBDATHREAD
#define QT_UTILS_QLAMBDATHREAD

#include <QThread>
#include <functional>

class QLambdaThread : public QObject {
  Q_OBJECT
public:
  QLambdaThread(QThread* thread, std::function<void()> f);
private slots:
  void start();
private:
  std::function<void()> f;
};

#endif