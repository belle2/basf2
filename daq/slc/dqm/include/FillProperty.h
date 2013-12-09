#ifndef _Belle2_FillProperty_hh
#define _Belle2_FillProperty_hh

#include "daq/slc/dqm/Property.h"

#include <string>

namespace Belle2 {

  class FillProperty : public Property {

  private:
    double _transparency;

  public:
    FillProperty(const MonColor& color,
                 double transparency = -1) throw();
    FillProperty(double transparency = -1) throw();
    virtual ~FillProperty() throw() {}

  public:
    double getTransparency() const throw() { return _transparency; }
    void setTransparency(double transparency) throw() {
      _transparency = transparency;
    }
    virtual std::string toString() const throw();

  };

};

#endif
