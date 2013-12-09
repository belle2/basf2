#include "daq/slc/dqm/Shape.h"

using namespace Belle2;

Shape::Shape() throw()
  : MonXMLElement(""), _line_pro(NULL), _fill_pro(NULL), _font_pro(NULL) {}

Shape::Shape(const std::string& name) throw()
  : MonXMLElement(name), _line_pro(NULL), _fill_pro(NULL), _font_pro(NULL) {}

Shape::~Shape() throw()
{
  if (_line_pro != NULL) delete _line_pro;
  if (_font_pro != NULL) delete _font_pro;
  if (_fill_pro != NULL) delete _fill_pro;
}
