#ifndef _Belle2_Shape_h
#define _Belle2_Shape_h

#include "daq/slc/dqm/MonXMLElement.h"
#include "daq/slc/dqm/FillProperty.h"
#include "daq/slc/dqm/LineProperty.h"
#include "daq/slc/dqm/FontProperty.h"
#include "daq/slc/dqm/MonColor.h"

#include <string>

namespace Belle2 {

  class Shape : public MonXMLElement {

  public:
    Shape() throw();
    Shape(const std::string& name) throw();
    virtual ~Shape() throw();

  public:
    LineProperty* getLine() throw() { return m_line_pro; }
    FillProperty* getFill() throw() { return m_fill_pro; }
    FontProperty* getFont() throw() { return m_font_pro; }
    void setLine(LineProperty* pro) throw() { m_line_pro = pro; }
    void setFill(FillProperty* pro) throw() { m_fill_pro = pro; }
    void setFont(FontProperty* pro) throw() { m_font_pro = pro; }

  protected:
    LineProperty* m_line_pro;
    FillProperty* m_fill_pro;
    FontProperty* m_font_pro;

  };

};

#endif
