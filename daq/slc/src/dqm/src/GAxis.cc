#include "dqm/GAxis.h"

#include "base/StringUtil.h"

#include <sstream>

using namespace Belle2;

GAxis::GAxis(const std::string& name, const std::string& title,
             int nbin, double min, double max)
  : Axis(nbin, min, max, title), Shape(name),
    _tick_line_pro(NULL), _tick_font_pro(NULL), _label_font_pro(NULL),
    _log_scale(false), _title_offset(-1), _label_offset(-1),
    _time_format(""), _position(""), _tick_length(-1),
    _x(-1), _y(-1), _padding_x(-1), _padding_y(-1) {}

GAxis::~GAxis() throw() {}

void GAxis::setTickLine(LineProperty* pro) throw()
{
  _tick_line_pro = pro;
  _tick_line_pro->setLabel("tick-");
}

void GAxis::setTickFont(FontProperty* pro) throw()
{
  _tick_font_pro = pro;
  _tick_font_pro->setLabel("tick-");
}

void GAxis::setLabelFont(FontProperty* pro) throw()
{
  _label_font_pro = pro;
  _label_font_pro->setLabel("label-");
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
    _labels = s1.str();
  } else {
    _labels = "";
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
    _labels = s1.str();
  } else {
    _labels = "";
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
  _color_pattern = ss.str();
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
  _color_pattern = ss.str();
}

std::string GAxis::toXML() const throw()
{
  std::stringstream ss;
  if (_tag.size() == 0) ss << "<axis ";
  else ss << "<" << _tag << " ";
  if (_name.size() > 0) ss << "name='" << _name << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  if (isFixedMin()) ss << "minimum='" << getMin() << "' ";
  if (isFixedMax()) ss << "maximum='" << getMax() << "' ";
  if (_labels.size() > 0) ss << "label='" << _labels << "' ";
  if (_color_pattern.size() > 0) ss << "color-pattern='" << _color_pattern << "' ";
  if (_log_scale) ss << "log-scale='on' ";
  if (_title_offset >= 0) ss << "title-offset='" << _title_offset << "' ";
  if (_label_offset >= 0) ss << "label-offset='" << _label_offset << "' ";
  if (_label_font_pro != NULL) ss << _label_font_pro->toString();
  if (_time_format.size() > 0) ss << "time-format='" << _time_format << "' ";
  if (_position.size() > 0) ss << "position='" << _position << "' ";
  if (_tick_length >= 0) ss << "tick-length='" << _tick_length << "' ";
  if (_tick_line_pro != NULL) ss << _tick_line_pro->toString();
  if (_tick_font_pro != NULL) ss << _tick_font_pro->toString();
  if (_x >= 0) ss << "x='" << _x << "' ";
  if (_y >= 0) ss << "y='" << _y << "' ";
  if (_padding_x >= 0) ss << "padding-x='" << _padding_x << "' ";
  if (_padding_y >= 0) ss << "padding-y='" << _padding_y << "' ";
  ss << ">" << std::endl;

  if (_title.size() > 0) {
    ss << "<title><![CDATA["
       << Belle2::replace(_title, "\n", "<br/>")
       << "]]></title>" << std::endl;
  }
  if (_tag.size() == 0) ss << "</axis>";
  else ss << "</" << _tag << ">";
  ss << std::endl;
  return ss.str();
}
