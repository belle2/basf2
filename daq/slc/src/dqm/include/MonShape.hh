#ifndef _B2DQM_MonShape_hh
#define _B2DQM_MonShape_hh

#include "MonObject.hh"
#include "MonColor.hh"

namespace B2DQM {

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
      _fill_color = MonColor(r, g, b); 
    }
    void setFillColor(const MonColor& color) throw() { _fill_color = color; }
    void setLineColor(int r, int g, int b) throw() {
      _line_color = MonColor(r, g, b); 
    }
    void setLineColor(const MonColor& color) throw() { _line_color = color; }
    const MonColor& getFillColor() const throw() { return _fill_color; }
    const MonColor& getLineColor() const throw() { return _line_color; }
    virtual std::string getTag() const throw() { return ELEMENT_TAG; }
    virtual std::string toXML() const throw();
    virtual void writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void writeContents(B2DAQ::Writer& writer) const throw(B2DAQ::IOException);
    virtual void readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    virtual void readContents(B2DAQ::Reader& reader) throw(B2DAQ::IOException);

  protected:
    MonColor _fill_color;
    MonColor _line_color;
    float _x, _y, _width, _height;

  };

};

#endif
