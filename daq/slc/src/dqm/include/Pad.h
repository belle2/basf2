#ifndef _Belle2_Pad_hh
#define _Belle2_Pad_hh

#include "dqm/Rect.h"

namespace Belle2 {

  class Pad : public Rect {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Pad(const std::string name = "",
        float x = 0.16, float y = 0.16,
        float width = 0.68, float height = 0.68) throw()
      : Rect(name, x, y, width, height) {}
    virtual ~Pad() throw() {}

  public:
    virtual std::string toXML() const throw();

  };

};

#endif
