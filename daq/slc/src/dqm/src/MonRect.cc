#include "MonRect.hh"

using namespace B2DQM;

void MonRect::reset() throw()
{
  _x = _y = 0;
  _width = _height = 1;
}

void MonRect::readConfig(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonObject::readConfig(reader);
}

void MonRect::readContents(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonShape::readContents(reader);
  _x = reader.readFloat();
  _y = reader.readFloat();
  _width = reader.readFloat();
  _height = reader.readFloat();
}

void MonRect::writeConfig(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonObject::writeConfig(writer);
}

void MonRect::writeContents(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(_x);
  writer.writeFloat(_y);
  writer.writeFloat(_width);
  writer.writeFloat(_height);
}
