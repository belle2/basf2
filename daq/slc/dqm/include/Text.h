#ifndef _Belle2_Text_hh
#define _Belle2_Text_hh

#include "daq/slc/dqm/Shape.h"

namespace Belle2 {

  class Text : public Shape {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Text(const std::string& name = "",
         const std::string& text = "", float x = 0, float y = 0) throw()
      : Shape(name), _text(text), _x(x), _y(y) {}
    virtual ~Text() throw() {}

  private:
    std::string _text;
    float _x, _y;

  public:
    void setX(float x) throw() { _x = x; }
    void setY(float y) throw() { _y = y; }
    void setText(const std::string& text) throw() { _text = text; }
    float getX() const throw() { return _x; }
    float getY() const throw() { return _y; }
    const std::string& getText() const throw() { return _text; }
    std::string getTag() const throw() { return ELEMENT_TAG; }
    std::string toXML() const throw();

  };

};

#endif
