#include "dqm/MonPieTable.h"

using namespace Belle2;

void MonPieTable::reset() throw()
{
  _array.setAll(0);
}

void MonPieTable::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
  _array = FloatArray(reader.readInt());
}

void MonPieTable::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonShape::readContents(reader);
  _array.readObject(reader);
}

void MonPieTable::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
  writer.writeInt((int)_array.size());
}

void MonPieTable::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonShape::writeContents(writer);
  _array.writeObject(writer);
}
