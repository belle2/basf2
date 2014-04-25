#include "daq/slc/dqm/MonObject.h"

#include <sstream>

using namespace Belle2;

MonObject::MonObject() throw()
  : Shape("none"), m_updated(false),
    m_update_id(-1), m_position_id(0), m_tab_id(0) {}

MonObject::MonObject(const std::string& name)
throw() : Shape(name), m_updated(false),
  m_update_id(-1), m_position_id(0), m_tab_id(0) {}

MonObject::~MonObject() throw() {}

void MonObject::readObject(Reader& reader)
throw(IOException)
{
  readConfig(reader);
  readContents(reader);
}

void MonObject::readConfig(Reader& reader)
throw(IOException)
{
  m_position_id = reader.readChar();
  m_tab_id = reader.readChar();
  m_name = reader.readString();
}

void MonObject::writeObject(Writer& writer)
const throw(IOException)
{
  writeConfig(writer);
  writeContents(writer);
}

void MonObject::writeConfig(Writer& writer)
const throw(IOException)
{
  writer.writeChar(m_position_id);
  writer.writeChar(m_tab_id);
  writer.writeString(m_name);
}

std::string MonObject::toString() const throw()
{
  std::stringstream ss;
  ss << "<mon-object data-type='" << getDataType() << "' "
     << "name='" << m_name << "' "
     << "update-id='" << m_update_id << "' "
     << "tab='" << (int)m_tab_id << "' "
     << "position='" << (int)m_position_id << "' />";
  return ss.str();
}

