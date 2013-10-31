#include "Histo1.h"

using namespace Belle2;

Histo1::Histo1() throw(): Histo() {}

Histo1::Histo1(const Histo1& h) throw(): Histo(h) {}

Histo1::Histo1(const std::string& name, const std::string& title,
               int nbinx, double min, double max)
throw() : Histo(name, title)
{
  _axis_x.set(nbinx, min, max);
}

void Histo1::reset() throw()
{
  setUpdateId(0);
  _data->setAll(0);
}

void Histo1::resetAxis(int nbinx, double xmin, double xmax)
{
  _axis_x.set(nbinx, xmin, xmax);
  _data->resize(_axis_x.getNbins() + 2);
}

double Histo1::getBinContent(int nx) const throw()
{
  return _data->get(nx + 1);
}

double Histo1::getOverFlow() const throw()
{
  return _data->get(_axis_x.getNbins() + 1);
}

double Histo1::getUnderFlow() const throw()
{
  return _data->get(0);
}

void Histo1::setBinContent(int nx, double data) throw()
{
  _data->set(nx + 1, data);
}

void Histo1::setOverFlow(double data) throw()
{
  _data->set(_axis_x.getNbins() + 1, data);
}

void Histo1::setUnderFlow(double data) throw()
{
  _data->set(0, data);
}

double Histo1::getMaximum() const throw()
{
  return _axis_y.getMax();
}

double Histo1::getMinimum() const throw()
{
  return _axis_y.getMin();
}

void Histo1::setMaximum(double data) throw()
{
  if (!_axis_y.isFixedMax()) _axis_y.setMax(data);
}

void Histo1::setMinimum(double data) throw()
{
  if (!_axis_y.isFixedMin()) _axis_y.setMin(data);
}

void Histo1::fixMaximum(double data) throw()
{
  _axis_y.fixMax(true);
  _axis_y.setMax(data);
}

void Histo1::fixMinimum(double data) throw()
{
  _axis_y.fixMin(true);
  _axis_y.setMin(data);
}

void Histo1::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  Histo::readConfig(reader);
  _axis_x.readObject(reader);
  _axis_y.setTitle(reader.readString());
  _data->resize(_axis_x.getNbins() + 2);
}

void Histo1::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  Histo::writeConfig(writer);
  _axis_x.writeObject(writer);
  writer.writeString(_axis_y.getTitle());
}
