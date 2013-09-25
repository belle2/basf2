#ifndef _B2DQM_MonLabel_hh
#define _B2DQM_MonLabel_hh

#include "MonShape.hh"
#include "CharArray.hh"

namespace B2DQM {

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
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);

    virtual std::string toString() const throw();
    virtual std::string toXML() const throw();

  private:
    MonColor _font_color;
    CharArray _text;
    std::string _align;

  };

};

#endif
