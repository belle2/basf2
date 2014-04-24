#include "daq/slc/dqm/MonLine.h"

using namespace Belle2;

void MonLine::reset() throw()
{
  _x1 = _y1 = 0;
  _x2 = _y2 = 1;
}

void MonLine::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
}

void MonLine::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  MonShape::readContents(reader);
  _x1 = reader.readFloat();
  _y1 = reader.readFloat();
  _x2 = reader.readFloat();
  _y2 = reader.readFloat();
}

void MonLine::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
}

void MonLine::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  MonShape::writeContents(writer);
  writer.writeFloat(_x1);
  writer.writeFloat(_y1);
  writer.writeFloat(_x2);
  writer.writeFloat(_y2);
}
