#include "daq/slc/dqm/MonShape.h"

#include <sstream>

using namespace Belle2;

const std::string MonShape::ELEMENT_TAG = "mon-shape";

MonShape::MonShape() :
  MonObject(""),
  m_x(-1), m_y(-1), m_width(-1), m_height(-1) {}

MonShape::MonShape(const std::string& name)
  : MonObject(name),
    m_x(-1), m_y(-1), m_width(-1), m_height(-1) {}

std::string MonShape::toXML() const throw()
{
  std::stringstream ss;
  ss << "<" << ELEMENT_TAG << " "
     << "name='" << m_name << "' ";
  if (m_line_pro != NULL) ss << m_line_pro->toString();
  if (m_fill_pro != NULL) ss << m_fill_pro->toString();
  if (m_font_pro != NULL) ss << m_font_pro->toString();
  if (m_x >= 0 && m_y >= 0 && m_width >= 0 && m_height >= 0) {
    ss << "bounds='" << m_x << "," << m_y << ","
       << m_width << "," << m_height << "' ";
  }
  ss << "/>\n";
  return ss.str();
}

void MonShape::setBounds(float x, float y, float width, float height) throw()
{
  m_x = x;
  m_y = y;
  m_width = width;
  m_height = height;
}

void MonShape::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
}

void MonShape::writeContents(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  writer.writeShort((short)m_fill_color.getRed());
  writer.writeShort((short)m_fill_color.getGreen());
  writer.writeShort((short)m_fill_color.getBlue());
  writer.writeShort((short)m_line_color.getRed());
  writer.writeShort((short)m_line_color.getGreen());
  writer.writeShort((short)m_line_color.getBlue());
}

void MonShape::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
}

void MonShape::readContents(Belle2::Reader& reader) throw(Belle2::IOException)
{
  short red = reader.readShort();
  short green = reader.readShort();
  short blue = reader.readShort();
  m_fill_color.setRGB(red, green, blue);
  red = reader.readShort();
  green = reader.readShort();
  blue = reader.readShort();
  m_line_color.setRGB(red, green, blue);
}

