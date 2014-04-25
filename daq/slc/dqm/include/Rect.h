#ifndef _Belle2_Rect_h
#define _Belle2_Rect_h

#include "daq/slc/dqm/Shape.h"

namespace Belle2 {

  class Rect : public Shape {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Rect(const std::string& name = "",
         float x = 0, float y = 0, float width = 1, float height = 1) throw()
      : Shape(name), m_x(x), m_y(y),
        m_width(width), m_height(height) {}
    virtual ~Rect() throw() {}

  public:
    void setBounds(float x, float y, float width, float height) throw();
    void setX(float x) throw() { m_x = x; }
    void setWidth(float width) throw() { m_width = width; }
    void setY(float y) throw() { m_y = y; }
    void setHeight(float height) throw() { m_height = height; }
    float getX() const throw() { return m_x; }
    float getY() const throw() { return m_y; }
    float getWidth() const throw() { return m_width; }
    float getHeight() const throw() { return m_height; }
    std::string getTag() const throw() { return ELEMENT_TAG; }
    std::string toXML() const throw();

  protected:
    float m_x, m_y, m_width, m_height;

  };

};

#endif
