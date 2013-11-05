#ifndef _Belle2_RefObject_hh
#define _Belle2_RefObject_hh

#include "dqm/Shape.h"
#include "dqm/Histo.h"

namespace Belle2 {

  class RefObject : public Shape {

  protected:
    Histo* _histo;

  public:
    static const std::string ELEMENT_TAG;

  public:
    RefObject() throw() : _histo(NULL) {};
    RefObject(Histo* histo) throw() : _histo(histo) {}
    virtual ~RefObject() throw() {}

  public:
    void setObject(Histo* histo) throw() {
      _histo = histo;
    }
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  };

};

#endif
