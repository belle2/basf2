#ifndef _Belle2_MonShape_h
#define _Belle2_MonShape_h

#include "daq/slc/dqm/MonObject.h"
#include "daq/slc/dqm/MonColor.h"

namespace Belle2 {

  class MonShape : public MonObject {

  public:
    static const std::string ELEMENT_TAG;

  public:
    MonShape();
    MonShape(const std::string& name);
    virtual ~MonShape() throw() {}

  public:
    void setBounds(float x, float y, float width, float height) throw();
    void setFillColor(int r, int g, int b) throw() {
      m_fill_color = MonColor(r, g, b);
    }
    void setFillColor(const MonColor& color) throw() { m_fill_color = color; }
    void setLineColor(int r, int g, int b) throw() {
      m_line_color = MonColor(r, g, b);
    }
    void setLineColor(const MonColor& color) throw() { m_line_color = color; }
    const MonColor& getFillColor() const throw() { return m_fill_color; }
    const MonColor& getLineColor() const throw() { return m_line_color; }
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException);
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException);

  protected:
    MonColor m_fill_color;
    MonColor m_line_color;
    float m_x, m_y, m_width, m_height;

  };

};

#endif
