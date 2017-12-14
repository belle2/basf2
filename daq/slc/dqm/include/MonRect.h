#ifndef _Belle2_MonRect_h
#define _Belle2_MonRect_h

#include "daq/slc/dqm/MonShape.h"

namespace Belle2 {

  class MonRect : public MonShape {

  public:
    MonRect(std::string name) : MonShape(name) {}
    MonRect() : MonShape() {}
    virtual ~MonRect() throw() {}

  public:
    virtual void reset() throw();
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException);
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException);
    virtual std::string getDataType() const throw() { return "MRT"; }

  public:
    void setX(float x) { m_x = x; }
    float getX() { return m_x; }
    void setWidth(float width) { m_width = width; }
    float getWidth() { return m_width; }
    void setY(float y) { m_y = y; }
    float getY() { return m_y; }
    void setHeight(float height) { m_height = height; }
    float getHeight() { return m_height; }

  };

};

#endif
