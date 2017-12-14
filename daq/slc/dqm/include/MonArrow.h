#ifndef _Belle2_MonArrow_h
#define _Belle2_MonArrow_h

#include "daq/slc/dqm/MonShape.h"

namespace Belle2 {

  class MonArrow : public MonShape {

  public:
    static const std::string ELEMENT_TAG;
    static const int DIRECT_RIGHT = 0;
    static const int DIRECT_LEFT = 1;
    static const int DIRECT_UP = 2;
    static const int DIRECT_DOWN = 3;

  public:
    MonArrow(std::string name) : MonShape(name) {}
    MonArrow() : MonShape() {}
    virtual ~MonArrow() throw() {}

  public:
    virtual void reset() throw() {}
    virtual std::string toXML() const throw();
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string getDataType() const throw() { return "MAW"; }

  public:
    void setPositionX(float x) { m_x = x; }
    float getPositionX() { return m_x; }
    void setPositionY(float y) { m_y = y; }
    float getPositionY() { return m_y; }
    void setWidth(float width) { m_width = width; }
    float getWidth() { return m_width; }
    void setLength(float length) { m_height = length; }
    float getLength() { return m_height; }
    void setHeadWidth(float width) { m_head_width = width; }
    float getHeadWidth() { return m_head_width; }
    void setHeadLength(float length) { m_head_length = length; }
    float getHeadLength() { return m_head_length; }
    void setDirection(int direction) { m_direction = direction; }
    int getDirection() const { return  m_direction; }

  private:
    float m_head_width;
    float m_head_length;
    int m_direction;

  };

};

#endif
