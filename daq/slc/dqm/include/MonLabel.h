#ifndef _Belle2_MonLabel_h
#define _Belle2_MonLabel_h

#include "daq/slc/dqm/MonShape.h"
#include "daq/slc/dqm/CharArray.h"

namespace Belle2 {

  class MonLabel : public MonShape {

  public:
    MonLabel();
    MonLabel(const std::string& name, int max);
    virtual ~MonLabel() throw() {}

  public:
    virtual std::string getDataType() const throw() { return "MLB"; }
    virtual void reset() throw() {}
    void setFontColor(int r, int g, int b) throw() {
      m_font_color = MonColor(r, g, b);
    }
    void setFontColor(const MonColor& color) throw() { m_font_color = color; }
    MonColor& getFontColor() throw() { return m_font_color; }
    const MonColor& getFontColor() const throw() { return m_font_color; }
    std::string getTextAlign() const throw() { return m_align; }
    void setTextAlign(const std::string& align) throw() { m_align = align; }
    void setText(const std::string& text) throw();
    std::string getText() const throw();

  public:
    virtual void writeConfig(Writer& writer) const throw(IOException);
    virtual void writeContents(Writer& writer) const throw(IOException);
    virtual void readConfig(Reader& reader) throw(IOException);
    virtual void readContents(Reader& reader) throw(IOException);

    virtual std::string toString() const throw();
    virtual std::string toXML() const throw();

  private:
    MonColor m_font_color;
    CharArray m_text;
    std::string m_align;

  };

};

#endif
