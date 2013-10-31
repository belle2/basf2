#include "MonLabel.h"

#include <cstring>
#include <sstream>

using namespace Belle2;

MonLabel::MonLabel()
  : MonShape(), _font_color(MonColor::BLACK), _align("center") {}

MonLabel::MonLabel(const std::string& name, int max)
  : MonShape(name), _font_color(MonColor::BLACK),
    _text(max), _align("center") {}

void MonLabel::setText(const std::string& text) throw()
{
  memset(_text.ptr(), '\0', _text.size());
  strncpy((char*)_text.ptr(), text.c_str(), _text.size());
}

std::string MonLabel::getText() const throw()
{
  return std::string((const char*)_text.ptr());
}

void MonLabel::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonShape::writeConfig(writer);
  writer.writeInt(_text.size());
}

void MonLabel::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonShape::writeContents(writer);
  writer.writeShort((short)_font_color.getRed());
  writer.writeShort((short)_font_color.getGreen());
  writer.writeShort((short)_font_color.getBlue());
  _text.writeObject(writer);
}

void MonLabel::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonShape::readConfig(reader);
  _text.resize(reader.readInt());
  _text.setAll('\0');
}

void MonLabel::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonShape::readContents(reader);
  short red = reader.readShort();
  short green = reader.readShort();
  short blue = reader.readShort();
  _font_color.setRGB(red, green, blue);
  _text.readObject(reader);
}

std::string MonLabel::toString() const throw()
{
  std::stringstream ss;
  ss << "<mon-object data-type='" << getDataType() << "' "
     << "name='" << _name << "' "
     << "update-id='" << _update_id << "' "
     << "tab='" << (int) _tab_id << "' "
     << "position='" << (int) _position_id << "' "
     << "text='" << getText() << "' />";
  return ss.str();
}

std::string MonLabel::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << _name << "' ";
  if (_line_pro != NULL) ss << _line_pro->toString();
  if (_fill_pro != NULL) ss << _fill_pro->toString();
  if (_font_pro != NULL) ss << _font_pro->toString();
  if (_x >= 0 && _y >= 0 && _width >= 0 && _height >= 0) {
    ss << "bounds='" << _x << "," << _y << ","
       << _width << "," << _height << "' ";
  }
  ss << "text-align='" << _align << "' "
     << "/>\n";
  return ss.str();
}

