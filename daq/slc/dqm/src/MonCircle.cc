#include "daq/slc/dqm/MonCircle.h"

using namespace Belle2;

void MonCircle::reset() throw()
{
  m_x = m_y = 0;
  m_r_x = m_r_y = 0.5;
}

void MonCircle::readConfig(Reader& reader)
throw(IOException)
{
  MonObject::readConfig(reader);
}

void MonCircle::readContents(Reader& reader)
throw(IOException)
{
  MonShape::readContents(reader);
  m_x = reader.readFloat();
  m_y = reader.readFloat();
  m_r_x = reader.readFloat();
  m_r_y = reader.readFloat();
}

void MonCircle::writeConfig(Writer& writer)
const throw(IOException)
{
  MonObject::writeConfig(writer);
}

void MonCircle::writeContents(Writer& writer)
const throw(IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(m_x);
  writer.writeFloat(m_y);
  writer.writeFloat(m_r_x);
  writer.writeFloat(m_r_y);
}
