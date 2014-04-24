#ifndef _Belle2_MonArrow_hh
#define _Belle2_MonArrow_hh

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
    void setPositionX(float x) { _x = x; }
    float getPositionX() { return _x; }
    void setPositionY(float y) { _y = y; }
    float getPositionY() { return _y; }
    void setWidth(float width) { _width = width; }
    float getWidth() { return _width; }
    void setLength(float length) { _height = length; }
    float getLength() { return _height; }
    void setHeadWidth(float width) { _head_width = width; }
    float getHeadWidth() { return _head_width; }
    void setHeadLength(float length) { _head_length = length; }
    float getHeadLength() { return _head_length; }
    void setDirection(int direction) { _direction = direction; }
    int getDirection() const { return  _direction; }

  private:
    float _head_width;
    float _head_length;
    int _direction;

  };

};

#endif
