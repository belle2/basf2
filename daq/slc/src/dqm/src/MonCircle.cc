#include "MonCircle.hh"

using namespace B2DQM;

void MonCircle::reset() throw()
{
  _x = _y = 0;
  _r_x = _r_y = 0.5;
}

void MonCircle::readConfig(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonObject::readConfig(reader);
}

void MonCircle::readContents(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  MonShape::readContents(reader);
  _x = reader.readFloat();
  _y = reader.readFloat();
  _r_x = reader.readFloat();
  _r_y = reader.readFloat();
}

void MonCircle::writeConfig(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonObject::writeConfig(writer);
}

void MonCircle::writeContents(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(_x);
  writer.writeFloat(_y);
  writer.writeFloat(_r_x);
  writer.writeFloat(_r_y);
}
