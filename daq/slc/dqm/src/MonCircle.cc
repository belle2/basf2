#include "daq/slc/dqm/MonCircle.h"

using namespace Belle2;

void MonCircle::reset() throw()
{
  _x = _y = 0;
  _r_x = _r_y = 0.5;
}

void MonCircle::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
}

void MonCircle::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonShape::readContents(reader);
  _x = reader.readFloat();
  _y = reader.readFloat();
  _r_x = reader.readFloat();
  _r_y = reader.readFloat();
}

void MonCircle::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
}

void MonCircle::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(_x);
  writer.writeFloat(_y);
  writer.writeFloat(_r_x);
  writer.writeFloat(_r_y);
}
