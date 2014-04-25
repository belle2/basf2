#ifndef _Belle2_LineProperty_h
#define _Belle2_LineProperty_h

#include "daq/slc/dqm/Property.h"

#include <string>

namespace Belle2 {

  class LineProperty : public Property {

  public:
    LineProperty(const MonColor& color,
                 int width = -1) throw();
    LineProperty(int width = -1) throw();
    virtual ~LineProperty() throw() {}

  public:
    int getWidth() const throw() { return m_width; }
    void setWidth(double width) throw() { m_width = width; }
    virtual std::string toString() const throw();

  private:
    int m_width;

  };

};

#endif
