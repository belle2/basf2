#ifndef _Belle2_Shape_hh
#define _Belle2_Shape_hh

#include "daq/slc/dqm/MonXMLElement.h"
#include "daq/slc/dqm/FillProperty.h"
#include "daq/slc/dqm/LineProperty.h"
#include "daq/slc/dqm/FontProperty.h"
#include "daq/slc/dqm/MonColor.h"

#include <string>

namespace Belle2 {

  class Shape : public MonXMLElement {

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
