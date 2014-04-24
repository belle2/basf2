#ifndef _Belle2_MonShape_hh
#define _Belle2_MonShape_hh

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
    virtual void writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void writeContents(Belle2::Writer& writer) const throw(Belle2::IOException);
    virtual void readConfig(Belle2::Reader& reader) throw(Belle2::IOException);
    virtual void readContents(Belle2::Reader& reader) throw(Belle2::IOException);

  protected:
    MonColor _fill_color;
    MonColor _line_color;
    float _x, _y, _width, _height;

  };

};

#endif
