#include "daq/slc/dqm/MonRect.h"

using namespace Belle2;

void MonRect::reset() throw()
{
  _x = _y = 0;
  _width = _height = 1;
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
  _x = reader.readFloat();
  _y = reader.readFloat();
  _width = reader.readFloat();
  _height = reader.readFloat();
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
  writer.writeFloat(_x);
  writer.writeFloat(_y);
  writer.writeFloat(_width);
  writer.writeFloat(_height);
}
