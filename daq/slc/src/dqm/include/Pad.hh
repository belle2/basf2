#ifndef _B2DQM_Pad_hh
#define _B2DQM_Pad_hh

#include "Rect.hh"

namespace B2DQM {

  class Pad : public Rect {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Pad(const std::string name="", 
	float x = 0.16, float y = 0.16,
	float width = 0.68, float height = 0.68) throw()
      : Rect(name, x, y, width, height) {}
    virtual ~Pad() throw() {}

  public:
    virtual std::string toXML() const throw();

  };

};

#endif
