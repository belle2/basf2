#ifndef _B2DQM_MonRect_hh
#define _B2DQM_MonRect_hh

#include "MonShape.hh"

namespace B2DQM {

  class MonRect : public MonShape {
    
  public:
    MonRect(std::string name) : MonShape(name) {}
    MonRect() : MonShape() {}
    virtual ~MonRect() throw() {}
    
  public:
    virtual void reset() throw();
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual std::string getDataType() const throw() { return "MRT"; }

  public:
    void setX(float x) { _x = x; }
    float getX() { return _x; }
    void setWidth(float width) { _width = width; }
    float getWidth() { return _width; }
    void setY(float y) { _y = y; }
    float getY() { return _y; }
    void setHeight(float height) { _height = height; }
    float getHeight() { return _height; }
    
  };

};

#endif
