#include "daq/slc/dqm/MonLine.h"

using namespace Belle2;

void MonLine::reset() throw()
{
  m_x1 = m_y1 = 0;
  m_x2 = m_y2 = 1;
}

void MonLine::readConfig(Reader& reader)
throw(IOException)
{
  MonObject::readConfig(reader);
}

void MonLine::readContents(Reader& reader)
throw(IOException)
{
  MonShape::readContents(reader);
  m_x1 = reader.readFloat();
  m_y1 = reader.readFloat();
  m_x2 = reader.readFloat();
  m_y2 = reader.readFloat();
}

void MonLine::writeConfig(Writer& writer)
const throw(IOException)
{
  MonObject::writeConfig(writer);
}

void MonLine::writeContents(Writer& writer)
const throw(IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(m_x1);
  writer.writeFloat(m_y1);
  writer.writeFloat(m_x2);
  writer.writeFloat(m_y2);
}
