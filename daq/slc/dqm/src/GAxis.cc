#include "daq/slc/dqm/GAxis.h"

#include <daq/slc/base/StringUtil.h>

#include <sstream>

using namespace Belle2;

GAxis::GAxis(const std::string& name, const std::string& title,
             int nbin, double min, double max)
  : Axis(nbin, min, max, title), Shape(name),
    m_tick_line_pro(NULL), m_tick_font_pro(NULL), m_label_font_pro(NULL),
    m_log_scale(false), m_title_offset(-1), m_label_offset(-1),
    m_time_format(""), m_position(""), m_tick_length(-1),
    m_x(-1), m_y(-1), m_padding_x(-1), m_padding_y(-1) {}

GAxis::~GAxis() throw() {}

void GAxis::setTickLine(LineProperty* pro) throw()
{
  m_tick_line_pro = pro;
  m_tick_line_pro->setLabel("tick-");
}

void GAxis::setTickFont(FontProperty* pro) throw()
{
  m_tick_font_pro = pro;
  m_tick_font_pro->setLabel("tick-");
}

void GAxis::setLabelFont(FontProperty* pro) throw()
{
  m_label_font_pro = pro;
  m_label_font_pro->setLabel("label-");
}

void GAxis::setLabels(const std::vector<std::string>& label,
                      const std::vector<double>& value) throw()
{
  if (value.size() > 0 && label.size() > 0) {
    std::stringstream s1, s2;
    for (size_t n = 0;;) {
      s1 << label[n];
      s2 << value[n];
      n++;
      if (n >= value.size() || n >= label.size()) break;
      s1 << ",";
      s2 << ",";
    }
    s1 << ":" << s2.str();
    m_labels = s1.str();
  } else {
    m_labels = "";
  }
}

void GAxis::setLabels(int nlabels, const std::string* label,
                      const double* value) throw()
{
  if (nlabels > 0) {
    std::stringstream s1, s2;
    for (int n = 0;;) {
      s1 << label[n];
      s2 << value[n];
      n++;
      if (n >= nlabels) break;
      s1 << ",";
      s2 << ",";
    }
    s1 << ":" << s2.str();
    m_labels = s1.str();
  } else {
    m_labels = "";
  }
}

void GAxis::setColorPattern(const std::vector<MonColor>& pattern) throw()
{
  std::stringstream ss;
  for (size_t n = 0;;) {
    ss << pattern[n].toString();
    n++;
    if (n >= pattern.size()) break;
    ss << ",";
  }
  m_color_pattern = ss.str();
}

void GAxis::setColorPattern(int ncolors, const MonColor* pattern) throw()
{
  std::stringstream ss;
  for (int n = 0;;) {
    ss << pattern[n].toString();
    n++;
    if (n >= ncolors) break;
    ss << ",";
  }
  m_color_pattern = ss.str();
}

std::string GAxis::toXML() const throw()
{
  std::stringstream ss;
  if (m_tag.size() == 0) ss << "<axis ";
  else ss << "<" << m_tag << " ";
  if (m_name.size() > 0) ss << "name='" << m_name << "' ";
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  if (isFixedMin()) ss << "minimum='" << getMin() << "' ";
  if (isFixedMax()) ss << "maximum='" << getMax() << "' ";
  if (m_labels.size() > 0) ss << "label='" << m_labels << "' ";
  if (m_color_pattern.size() > 0) ss << "color-pattern='" << m_color_pattern << "' ";
  if (m_log_scale) ss << "log-scale='on' ";
  if (m_title_offset >= 0) ss << "title-offset='" << m_title_offset << "' ";
  if (m_label_offset >= 0) ss << "label-offset='" << m_label_offset << "' ";
  if (m_label_font_pro != NULL) ss << m_label_font_pro->toString();
  if (m_time_format.size() > 0) ss << "time-format='" << m_time_format << "' ";
  if (m_position.size() > 0) ss << "position='" << m_position << "' ";
  if (m_tick_length >= 0) ss << "tick-length='" << m_tick_length << "' ";
  if (m_tick_line_pro != NULL) ss << m_tick_line_pro->toString();
  if (m_tick_font_pro != NULL) ss << m_tick_font_pro->toString();
  if (m_x >= 0) ss << "x='" << m_x << "' ";
  if (m_y >= 0) ss << "y='" << m_y << "' ";
  if (m_padding_x >= 0) ss << "padding-x='" << m_padding_x << "' ";
  if (m_padding_y >= 0) ss << "padding-y='" << m_padding_y << "' ";
  ss << ">" << std::endl;

  if (m_title.size() > 0) {
    ss << "<title><![CDATA["
       << StringUtil::replace(m_title, "\n", "<br/>")
       << "]]></title>" << std::endl;
  }
  if (m_tag.size() == 0) ss << "</axis>";
  else ss << "</" << m_tag << ">";
  ss << std::endl;
  return ss.str();
}
