#ifndef _Belle2_Text_h
#define _Belle2_Text_h

#include "daq/slc/dqm/Shape.h"

namespace Belle2 {

  class Text : public Shape {

  public:
    static const std::string ELEMENT_TAG;

  public:
    Text(const std::string& name = "",
         const std::string& text = "", float x = 0, float y = 0) throw()
      : Shape(name), m_text(text), m_x(x), m_y(y) {}
    virtual ~Text() throw() {}

  public:
    void setX(float x) throw() { m_x = x; }
    void setY(float y) throw() { m_y = y; }
    void setText(const std::string& text) throw() { m_text = text; }
    float getX() const throw() { return m_x; }
    float getY() const throw() { return m_y; }
    const std::string& getText() const throw() { return m_text; }
    std::string getTag() const throw() { return ELEMENT_TAG; }
    std::string toXML() const throw();

  private:
    std::string m_text;
    float m_x, m_y;

  };

};

#endif
