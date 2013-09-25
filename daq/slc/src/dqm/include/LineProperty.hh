#ifndef _B2DQM_LineProperty_hh
#define _B2DQM_LineProperty_hh

#include "Property.hh"

#include <string>

namespace B2DQM {

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
