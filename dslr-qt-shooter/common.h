#ifndef DSLR_SHOOTER_COMMON_H
#define DSLR_SHOOTER_COMMON_H
#include <QMetaType>

enum MessageType {
  Info = 0,
  Warning = 1,
  Error = 2,
};
class ____RegisterMetaTypes {
public:
  ____RegisterMetaTypes() { qRegisterMetaType<MessageType>("MessageType"); }
};

static ____RegisterMetaTypes ____registerMetaTypes;
#endif