#ifndef _Belle2_MonRect_hh
#define _Belle2_MonRect_hh

#include "daq/slc/dqm/MonShape.h"

namespace Belle2 {

  class MonRect : public MonShape {

  public:
    MonRect(std::string name) : MonShape(name) {}
    MonRect() : MonShape() {}
    virtual ~MonRect() throw() {}

  public:
    virtual void reset() throw();
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
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
