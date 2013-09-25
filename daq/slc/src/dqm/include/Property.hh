#ifndef _B2DQM_Property_hh
#define _B2DQM_Property_hh

#include "MonColor.hh"

#include <string>

namespace B2DQM {

  class Property {

  protected:
    MonColor _color;
    std::string _label;
    bool _use_color;

  public:
    Property(const MonColor& color) throw();
    Property() throw();
    virtual ~Property() throw() {}
    
  public:
    const MonColor& getColor() const throw() { return _color; }
    void setColor(int r, int g, int b) throw() { 
      _use_color = true;
      _color.setRGB(r,g,b);
    }
    void setColor(const MonColor& color) throw() { 
      _use_color = true;
      _color = color;
    }
    void setLabel(const std::string& label) throw() {
      _label = label;
    }

  public:
    virtual std::string toString() const throw() = 0;

  };

};

#endif
