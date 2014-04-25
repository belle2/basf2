#ifndef _Belle2_FillProperty_h
#define _Belle2_FillProperty_h

#include "daq/slc/dqm/Property.h"

#include <string>

namespace Belle2 {

  class FillProperty : public Property {

  public:
    FillProperty(const MonColor& color,
                 double transparency = -1) throw();
    FillProperty(double transparency = -1) throw();
    virtual ~FillProperty() throw() {}

  public:
    double getTransparency() const throw() { return m_transparency; }
    void setTransparency(double transparency) throw() {
      m_transparency = transparency;
    }
    virtual std::string toString() const throw();

  private:
    double m_transparency;

  };

};

#endif
