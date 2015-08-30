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

typedef sequence<int, GP_OK, std::greater_equal<int>, std::shared_ptr<QMutexLocker>> gp_api;

#endif

