#ifndef _B2DQM_Shape_hh
#define _B2DQM_Shape_hh

#include "XMLElement.hh"
#include "FillProperty.hh"
#include "LineProperty.hh"
#include "FontProperty.hh"
#include "MonColor.hh"

#include <string>

namespace B2DQM {

  class Shape : public XMLElement {

  protected:
    LineProperty* _line_pro;
    FillProperty* _fill_pro;
    FontProperty* _font_pro;

  public:
    Shape() throw();
    Shape(const std::string& name) throw();
    virtual ~Shape() throw();

  public:
    LineProperty* getLine() throw() { return _line_pro; }
    FillProperty* getFill() throw() { return _fill_pro; }
    FontProperty* getFont() throw() { return _font_pro; }
    void setLine(LineProperty* pro) throw() { _line_pro = pro; }
    void setFill(FillProperty* pro) throw() { _fill_pro = pro; }
    void setFont(FontProperty* pro) throw() { _font_pro = pro; }

  };

};

#endif
