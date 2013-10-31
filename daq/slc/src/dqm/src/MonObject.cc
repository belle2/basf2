#include "MonObject.h"

#include <sstream>

using namespace Belle2;

MonObject::MonObject() throw()
  : Shape("none"), _updated(false),
    _update_id(-1), _position_id(0), _tab_id(0) {}

MonObject::MonObject(const std::string& name)
throw() : Shape(name), _updated(false),
  _update_id(-1), _position_id(0), _tab_id(0) {}

MonObject::~MonObject() throw() {}

void MonObject::readObject(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  readConfig(reader);
  readContents(reader);
}

void MonObject::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  _position_id = reader.readChar();
  _tab_id = reader.readChar();
  _name = reader.readString();
}

void MonObject::writeObject(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writeConfig(writer);
  writeContents(writer);
}

void MonObject::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeChar(_position_id);
  writer.writeChar(_tab_id);
  writer.writeString(_name);
}

std::string MonObject::toString() const throw()
{
  std::stringstream ss;
  ss << "<mon-object data-type='" << getDataType() << "' "
     << "name='" << _name << "' "
     << "update-id='" << _update_id << "' "
     << "tab='" << (int) _tab_id << "' "
     << "position='" << (int) _position_id << "' />";
  return ss.str();
}

