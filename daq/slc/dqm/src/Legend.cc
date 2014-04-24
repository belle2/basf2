#include "daq/slc/dqm/Legend.h"

#include <sstream>

using namespace Belle2;

const std::string Legend::ELEMENT_TAG = "legend";

void Legend::add(Histo* obj) throw()
{
  if (obj != NULL) {
    _object_v.push_back(obj);
  }
}

std::string Legend::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " ";
  if (_name.size() > 0) ss << "name='" << _name << "' ";
  if (_x >= 0 && _y >= 0 && _width >= 0 && _height >= 0) {
    ss << "x='" << _x << "' y='" << _y << "' "
       << "width='" << _width << "' height='" << _height << "' ";
  }
  if (_object_v.size() > 0) {
    ss << "list='";
    for (size_t n = 0;;) {
      ss << _object_v[n]->getName();
      n++;
      if (n >= _object_v.size()) break;
      ss << ",";
    }
    ss << "' ";
  }
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  ss << "/>" << std::endl;

  return ss.str();
}
