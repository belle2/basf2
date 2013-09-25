#include "Range.hh"

using namespace B2DQM;

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

void Range::readConfig(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  MonObject::readConfig(reader);
  setMinimum(reader.readDouble());
  setMaximum(reader.readDouble());
}

void Range::writeConfig(B2DAQ::Writer& writer) const throw(B2DAQ::IOException)
{
  MonObject::writeConfig(writer);
  writer.writeDouble(getMinimum());
  writer.writeDouble(getMaximum());
}

void Range::readUpdate(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{

}

void Range::writeUpdate(B2DAQ::Writer& writer) const throw(B2DAQ::IOException)
{

}

void Range::readUpdateFull(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{

}

void Range::writeUpdateFull(B2DAQ::Writer& writer) const throw(B2DAQ::IOException)
{

}

