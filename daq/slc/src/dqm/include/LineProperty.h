#ifndef _Belle2_LineProperty_hh
#define _Belle2_LineProperty_hh

#include "dqm/Property.h"

#include <string>

namespace Belle2 {

  class LineProperty : public Property {

  private:
    int _width;

  public:
    LineProperty(const MonColor& color,
                 int width = -1) throw();
    LineProperty(int width = -1) throw();
    virtual ~LineProperty() throw() {}

  public:
    int getWidth() const throw() { return _width; }
    void setWidth(double width) throw() { _width = width; }
    virtual std::string toString() const throw();

  };

};

#endif
