#include "daq/slc/dqm/MonLabel.h"

#include <cstring>
#include <sstream>

using namespace Belle2;

MonLabel::MonLabel()
  : MonShape(), m_font_color(MonColor::BLACK), m_align("center") {}

MonLabel::MonLabel(const std::string& name, int max)
  : MonShape(name), m_font_color(MonColor::BLACK),
    m_text(max), m_align("center") {}

void MonLabel::setText(const std::string& text) throw()
{
  memset(m_text.ptr(), '\0', m_text.size());
  strncpy((char*)m_text.ptr(), text.c_str(), m_text.size());
}

std::string MonLabel::getText() const throw()
{
  return std::string((const char*)m_text.ptr());
}

void MonLabel::writeConfig(Writer& writer)
const throw(IOException)
{
  MonShape::writeConfig(writer);
  writer.writeInt(m_text.size());
}

void MonLabel::writeContents(Writer& writer)
const throw(IOException)
{
  MonShape::writeContents(writer);
  writer.writeShort((short)m_font_color.getRed());
  writer.writeShort((short)m_font_color.getGreen());
  writer.writeShort((short)m_font_color.getBlue());
  m_text.writeObject(writer);
}

void MonLabel::readConfig(Reader& reader)
throw(IOException)
{
  MonShape::readConfig(reader);
  m_text.resize(reader.readInt());
  m_text.setAll('\0');
}

void MonLabel::readContents(Reader& reader)
throw(IOException)
{
  MonShape::readContents(reader);
  short red = reader.readShort();
  short green = reader.readShort();
  short blue = reader.readShort();
  m_font_color.setRGB(red, green, blue);
  m_text.readObject(reader);
}

std::string MonLabel::toString() const throw()
{
  std::stringstream ss;
  ss << "<mon-object data-type='" << getDataType() << "' "
     << "name='" << m_name << "' "
     << "update-id='" << m_update_id << "' "
     << "tab='" << (int) m_tab_id << "' "
     << "position='" << (int) m_position_id << "' "
     << "text='" << getText() << "' />";
  return ss.str();
}

std::string MonLabel::toXML() const throw()
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
  ss << "text-align='" << m_align << "' "
     << "/>\n";
  return ss.str();
}

