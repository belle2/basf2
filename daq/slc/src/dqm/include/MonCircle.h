#ifndef _Belle2_MonCircle_hh
#define _Belle2_MonCircle_hh

#include "MonShape.h"

namespace Belle2 {

  class MonCircle : public MonShape {

  public:
    MonCircle(std::string name) : MonShape(name) {}
    MonCircle() : MonShape() {}
    virtual ~MonCircle() throw() {}

  public:
    virtual void reset() throw();
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual std::string getDataType() const throw() { return "MCC"; }

  public:
    void setX(float x) { _x = x; }
    float getX() { return _x; }
    void setRX(float rx) { _r_x = rx; }
    float getRX() { return _r_x; }
    void setY(float y) { _y = y; }
    float getY() { return _y; }
    void setRY(float ry) { _r_y = ry; }
    float getRY() { return _r_y; }

  private:
    float _x, _y, _r_x, _r_y;

  };

};

#endif
