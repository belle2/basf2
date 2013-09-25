#include "MonLine.hh"

using namespace B2DQM;

void MonLine::reset() throw()
{
  _x1 = _y1 = 0;
  _x2 = _y2 = 1;
}

void MonLine::readConfig(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonObject::readConfig(reader);
}

void MonLine::readContents(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonShape::readContents(reader);
  _x1 = reader.readFloat();
  _y1 = reader.readFloat();
  _x2 = reader.readFloat();
  _y2 = reader.readFloat();
}

void MonLine::writeConfig(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonObject::writeConfig(writer);
}

void MonLine::writeContents(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(_x1);
  writer.writeFloat(_y1);
  writer.writeFloat(_x2);
  writer.writeFloat(_y2);
}
