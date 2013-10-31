#ifndef _Belle2_Rect_hh
#define _Belle2_Rect_hh

#include "Shape.h"

namespace Belle2 {

  class Rect : public Shape {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Rect(const std::string& name = "",
         float x = 0, float y = 0, float width = 1, float height = 1) throw()
      : Shape(name), _x(x), _y(y),
        _width(width), _height(height) {}
    virtual ~Rect() throw() {}

  public:
    void setBounds(float x, float y, float width, float height) throw();
    void setX(float x) throw() { _x = x; }
    void setWidth(float width) throw() { _width = width; }
    void setY(float y) throw() { _y = y; }
    void setHeight(float height) throw() { _height = height; }
    float getX() const throw() { return _x; }
    float getY() const throw() { return _y; }
    float getWidth() const throw() { return _width; }
    float getHeight() const throw() { return _height; }
    std::string getTag() const throw() { return ELEMENT_TAG; }
    std::string toXML() const throw();

  protected:
    float _x, _y, _width, _height;

  };

};

#endif
