#ifndef SEQUENCE_ELEMENT_H
#define SEQUENCE_ELEMENT_H

#include "imagingsequence.h"
#include <QQueue>
#include "Qt/strings.h"

struct SequenceElement {
  ImagingSequence::ptr imagingSequence;
  QString displayName;
  struct Wait {
    Wait(qint64 seconds = -1) : seconds(seconds) {}
    qint64 seconds;
    operator bool() const { return seconds > -1; }
    QString toString() const {
      if(seconds < 0)
	return "<invalid wait>";
      if(seconds == 0)
	return "wait for confirmation";
      return "wait for %1"_q % QTime{0,0,0}.addSecs(seconds).toString();
    }
  };
  Wait wait;
  operator QString() const { return toString(); }
  QString toString() const {
    return "%1: %2"_q % displayName % (imagingSequence ? imagingSequence->toString() : wait.toString());
  }
};

class Sequence : public QQueue<SequenceElement> {
public:
  Sequence() = default;
  Sequence(const std::initializer_list<SequenceElement> &elements) {
    for(auto element: elements)
      enqueue(element);
  }
};

inline QDebug operator <<(QDebug dbg, const SequenceElement &el) {
  dbg.nospace() << "SequenceElement {" << el.toString() << "}";
  return dbg.space();
}

#endif
