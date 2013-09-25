#ifndef _B2DQM_MonCircle_hh
#define _B2DQM_MonCircle_hh

#include "MonShape.hh"

namespace B2DQM {

  class MonCircle : public MonShape {
    
  public:
    MonCircle(std::string name) : MonShape(name) {}
    MonCircle() : MonShape() {}
    virtual ~MonCircle() throw() {}
    
  public:
    virtual void reset() throw();
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
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
