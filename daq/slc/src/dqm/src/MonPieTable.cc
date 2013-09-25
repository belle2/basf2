#include "MonPieTable.hh"

using namespace B2DQM;

void MonPieTable::reset() throw()
{
  _array.setAll(0);
}

void MonPieTable::readConfig(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonObject::readConfig(reader);
  _array = FloatArray(reader.readInt());
}

void MonPieTable::readContents(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonShape::readContents(reader);
  _array.readObject(reader);
}

void MonPieTable::writeConfig(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonObject::writeConfig(writer);
  writer.writeInt((int)_array.size());
}

void MonPieTable::writeContents(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonShape::writeContents(writer);
  _array.writeObject(writer);
}
