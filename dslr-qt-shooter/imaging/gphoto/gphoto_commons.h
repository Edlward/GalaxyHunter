#ifndef GULINUX_GPHOTO_COMMONS_H
#define GULINUX_GPHOTO_COMMONS_H
#include <string>
#include "utils/sequence.h"
#include <gphoto2/gphoto2.h>
#include <QMutex>

class GPhotoCameraInformation {
public:
  GPhotoCameraInformation(const std::string &name, const std::string &port, GPContext *context, QMutex &mutex) 
    : name(name), port(port), context(context), mutex(mutex) {}
  std::string name;
  std::string port;
  GPContext *context;
  QMutex &mutex;
};


class gp_api {
  typedef sequence<int, GP_OK, std::greater_equal<int>> api_sequence;
public:
  gp_api(QMutex &mutex, const std::list<api_sequence::run> &runs)
    : api{runs}, locker(&mutex) {}
  ~gp_api() = default;
  gp_api &on_error(api_sequence::on_error_f e) {
    api.on_error(e);
    return *this;
  }
  gp_api &run_last(std::function<void()> _run_last) {
    api.run_last(_run_last);
    return *this;
  }
private:
  QMutexLocker locker;
  api_sequence api;
};
#endif

