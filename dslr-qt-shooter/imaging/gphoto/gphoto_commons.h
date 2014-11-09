#ifndef GULINUX_GPHOTO_COMMONS_H
#define GULINUX_GPHOTO_COMMONS_H
#include <string>
#include "utils/sequence.h"
#include <gphoto2/gphoto2.h>
typedef sequence<int, GP_OK, std::greater_equal<int>> gp_api;

class GPhotoCameraInformation {
public:
  GPhotoCameraInformation(const std::string &name, const std::string &port, GPContext *context) 
    : name(name), port(port), context(context) {}
  std::string name;
  std::string port;
  GPContext *context;
};

#endif

