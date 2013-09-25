#ifndef _B2DQM_RefObject_hh
#define _B2DQM_RefObject_hh

#include "Shape.hh"
#include "Histo.hh"

namespace B2DQM {

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
