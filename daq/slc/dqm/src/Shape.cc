#include "daq/slc/dqm/Shape.h"

using namespace Belle2;

Shape::Shape() throw()
  : MonXMLElement(""), m_line_pro(NULL), m_fill_pro(NULL), m_font_pro(NULL) {}

Shape::Shape(const std::string& name) throw()
  : MonXMLElement(name), m_line_pro(NULL), m_fill_pro(NULL), m_font_pro(NULL) {}

Shape::~Shape() throw()
{
  if (m_line_pro != NULL) delete m_line_pro;
  if (m_font_pro != NULL) delete m_font_pro;
  if (m_fill_pro != NULL) delete m_fill_pro;
}
