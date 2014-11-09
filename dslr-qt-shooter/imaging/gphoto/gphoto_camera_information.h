#ifndef GULINUX_GPHOTO_CAMERA_INFORMATION
#define GULINUX_GPHOTO_CAMERA_INFORMATION
#include <string>
#include <gphoto2/gphoto2.h>

class GPhotoCameraInformation {
public:
  GPhotoCameraInformation(const std::string &name, const std::string &port, GPContext *context) 
    : name(name), port(port), context(context) {}
  std::string name;
  std::string port;
  GPContext *context;
};

#endif

