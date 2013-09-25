#include "MonObject.hh"

#include <sstream>

using namespace B2DQM;

MonObject::MonObject() throw()
  : Shape("none"), _updated(false),
    _update_id(-1), _position_id(0), _tab_id(0) {}

MonObject::MonObject(const std::string& name)
throw() : Shape(name), _updated(false),
  _update_id(-1), _position_id(0), _tab_id(0) {}

MonObject::~MonObject() throw() {}

void MonObject::readObject(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  readConfig(reader);
  readContents(reader);
}

void MonObject::readConfig(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  _position_id = reader.readChar();
  _tab_id = reader.readChar();
  _name = reader.readString();
}

void MonObject::writeObject(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  writeConfig(writer);
  writeContents(writer);
}

void MonObject::writeConfig(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
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

