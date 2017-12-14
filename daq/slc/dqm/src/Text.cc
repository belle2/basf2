#include "daq/slc/dqm/Text.h"

#include "daq/slc/base/StringUtil.h"

#include <sstream>

using namespace Belle2;

const std::string Text::ELEMENT_TAG = "text";

std::string Text::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (m_name.size() > 0) ss << "name='" << m_name << "' ";
  ss << "x='" << m_x << "' y='" << m_y << "' ";
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  ss << ">" << std::endl;
  if (m_text.size() > 0) {
    ss << "<value><![CDATA["
       << StringUtil::replace(m_text, "\n", "<br/>")
       << "]]></value>" << std::endl;
  }
  ss << "</" << ELEMENT_TAG << ">" << std::endl;

  return ss.str();
}
