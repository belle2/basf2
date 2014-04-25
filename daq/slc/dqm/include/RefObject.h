#ifndef _Belle2_RefObject_h
#define _Belle2_RefObject_h

#include "daq/slc/dqm/Shape.h"
#include "daq/slc/dqm/Histo.h"

namespace Belle2 {

  class RefObject : public Shape {

  public:
    static const std::string ELEMENT_TAG;

  public:
    RefObject() throw() : m_histo(NULL) {};
    RefObject(Histo* histo) throw() : m_histo(histo) {}
    virtual ~RefObject() throw() {}

  public:
    void setObject(Histo* histo) throw() {
      m_histo = histo;
    }
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();

  protected:
    Histo* m_histo;

  };

};

#endif
