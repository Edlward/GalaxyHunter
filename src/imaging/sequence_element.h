#ifndef SEQUENCE_ELEMENT_H
#define SEQUENCE_ELEMENT_H

#include "imagingsequence.h"
#include <QQueue>

struct SequenceElement {
  ImagingSequence::ptr imagingSequence;
  QString displayName;
};

class Sequence : public QQueue<SequenceElement> {
public:
  Sequence() = default;
  Sequence(const std::initializer_list<SequenceElement> &elements) {
    for(auto element: elements)
      enqueue(element);
  }
};
#endif
