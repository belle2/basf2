#include "daq/slc/dqm/MonPieTable.h"

using namespace Belle2;

void MonPieTable::reset() throw()
{
  m_array.setAll(0);
}

void MonPieTable::readConfig(Reader& reader)
throw(IOException)
{
  MonObject::readConfig(reader);
  m_array = FloatArray(reader.readInt());
}

void MonPieTable::readContents(Reader& reader)
throw(IOException)
{
  MonShape::readContents(reader);
  m_array.readObject(reader);
}

void MonPieTable::writeConfig(Writer& writer)
const throw(IOException)
{
  MonObject::writeConfig(writer);
  writer.writeInt((int)m_array.size());
}

void MonPieTable::writeContents(Writer& writer)
const throw(IOException)
{
  MonShape::writeContents(writer);
  m_array.writeObject(writer);
}
