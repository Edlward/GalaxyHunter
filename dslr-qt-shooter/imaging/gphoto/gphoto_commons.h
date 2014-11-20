#ifndef GULINUX_GPHOTO_COMMONS_H
#define GULINUX_GPHOTO_COMMONS_H
#include <string>
#include "utils/sequence.h"
#include <gphoto2/gphoto2.h>
#include <QMutex>
typedef sequence<int, GP_OK, std::greater_equal<int>> gp_api;

class GPhotoCameraInformation {
public:
  GPhotoCameraInformation(const std::string &name, const std::string &port, GPContext *context) 
    : name(name), port(port), context(context) {}
  std::string name;
  std::string port;
  GPContext *context;
};


class GPhotoAPICall {
public:
  GPhotoAPICall(const std::list<gp_api::run> &runs, QMutex &mutex)
    : api{runs}, locker(&mutex) {}
  ~GPhotoAPICall();
private:
  QMutexLocker locker;
  gp_api api;
};
#endif

