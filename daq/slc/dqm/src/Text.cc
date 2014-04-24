#include "daq/slc/dqm/Text.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

const std::string Text::ELEMENT_TAG = "text";

std::string Text::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (_name.size() > 0) ss << "name='" << _name << "' ";
  ss << "x='" << _x << "' y='" << _y << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  ss << ">" << std::endl;
  if (_text.size() > 0) {
    ss << "<value><![CDATA["
       << StringUtil::replace(_text, "\n", "<br/>")
       << "]]></value>" << std::endl;
  }
  ss << "</" << ELEMENT_TAG << ">" << std::endl;

  return ss.str();
}
