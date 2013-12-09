#ifndef _Belle2_MonLabel_hh
#define _Belle2_MonLabel_hh

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
      _font_color = MonColor(r, g, b);
    }
    void setFontColor(const MonColor& color) throw() { _font_color = color; }
    MonColor& getFontColor() throw() { return _font_color; }
    const MonColor& getFontColor() const throw() { return _font_color; }
    std::string getTextAlign() const throw() { return _align; }
    void setTextAlign(const std::string& align) throw() { _align = align; }
    void setText(const std::string& text) throw();
    std::string getText() const throw();

  public:
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);

    virtual std::string toString() const throw();
    virtual std::string toXML() const throw();

  private:
    MonColor _font_color;
    CharArray _text;
    std::string _align;

  };

};

#endif
