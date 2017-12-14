#include "daq/slc/dqm/MonRect.h"

using namespace Belle2;

void MonRect::reset() throw()
{
  m_x = m_y = 0;
  m_width = m_height = 1;
}

void MonRect::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
}

void MonRect::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonShape::readContents(reader);
  m_x = reader.readFloat();
  m_y = reader.readFloat();
  m_width = reader.readFloat();
  m_height = reader.readFloat();
}

void MonRect::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
}

void MonRect::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(m_x);
  writer.writeFloat(m_y);
  writer.writeFloat(m_width);
  writer.writeFloat(m_height);
}
