#include "dqm/Range.h"

using namespace Belle2;

Range::Range() : Histo1D("", "", 2, 0, 1) {}

Range::Range(const std::string& name) : Histo1D(name, name, 2, 0, 1) {}

Range::Range(const std::string& name, double min, double max) : Histo1D(name, name, 2, 0, 1)
{
  _data->set(0, min);
  _data->set(1, max);
}

Range::~Range() throw() {}

void Range::set(double min, double max) throw()
{
  _data->set(0, min);
  _data->set(1, max);
}

void Range::reset() throw() {}

void Range::fixMaximum(double data, bool fix) throw()
{
  _data->set(1, data);
}

void Range::fixMinimum(double data, bool fix) throw()
{
  _data->set(0, data);
}

int Range::getDim() const throw()
{
  return 0;
}

double Range::getMaximum() const throw()
{
  return _data->get(1);
}

double Range::getMinimum() const throw()
{
  return _data->get(0);
}

void Range::setMaximum(double data) throw()
{
  _data->set(1, data);
}

void Range::setMinimum(double data) throw()
{
  _data->set(0, data);
}

void Range::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  MonObject::readConfig(reader);
  setMinimum(reader.readDouble());
  setMaximum(reader.readDouble());
}

void Range::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  MonObject::writeConfig(writer);
  writer.writeDouble(getMinimum());
  writer.writeDouble(getMaximum());
}

void Range::readUpdate(Belle2::Reader& reader) throw(Belle2::IOException)
{

}

void Range::writeUpdate(Belle2::Writer& writer) const throw(Belle2::IOException)
{

}

void Range::readUpdateFull(Belle2::Reader& reader) throw(Belle2::IOException)
{

}

void Range::writeUpdateFull(Belle2::Writer& writer) const throw(Belle2::IOException)
{

}

