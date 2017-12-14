#ifndef _Belle2_MonCircle_h
#define _Belle2_MonCircle_h

#include "daq/slc/dqm/MonShape.h"

namespace Belle2 {

  class MonCircle : public MonShape {

  public:
    MonCircle(std::string name) : MonShape(name) {}
    MonCircle() : MonShape() {}
    virtual ~MonCircle() throw() {}

  public:
    virtual void reset() throw();
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException);
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException);
    virtual std::string getDataType() const throw() { return "MCC"; }

  public:
    void setX(float x) { m_x = x; }
    float getX() { return m_x; }
    void setRX(float rx) { m_r_x = rx; }
    float getRX() { return m_r_x; }
    void setY(float y) { m_y = y; }
    float getY() { return m_y; }
    void setRY(float ry) { m_r_y = ry; }
    float getRY() { return m_r_y; }

  private:
    float m_x, m_y, m_r_x, m_r_y;

  };

};

#endif
