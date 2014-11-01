#include "imaging_driver.h"

#ifdef IMAGING_gphoto2
#include "gphoto.h"
#warning Using gphoto2 backend
#endif
#ifdef IMAGING_testing
#include "testingimager.h"
#warning Using testing backend
#endif


ImagingDriver::ImagingDriver(QObject *parent) : QObject(parent) {
}

ImagingDriver *ImagingDriver::imagingDriver(QObject *parent) {
#ifdef IMAGING_gphoto2
    return new GPhoto(parent);
#endif
#ifdef IMAGING_testing
    return new TestingImagerDriver(parent);
#endif
}
