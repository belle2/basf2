#include "dqm/Histo2.h"

using namespace Belle2;

Histo2::Histo2() throw() : Histo() {}

Histo2::Histo2(const Histo2& h) throw() : Histo(h) {}

Histo2::Histo2(const std::string& name, const std::string& title,
               int nbinx, double xmin, double xmax,
               int nbiny, double ymin, double ymax) throw() : Histo(name, title)
{
  _axis_x.set(nbinx, xmin, xmax);
  _axis_y.set(nbiny, ymin, ymax);
}

void Histo2::reset() throw()
{
  setUpdateId(0);
  _data->setAll(0);
}

void Histo2::resetAxis(int nbinx, double xmin, double xmax,
                       int nbiny, double ymin, double ymax)
{
  _axis_x.set(nbinx, xmin, xmax);
  _axis_y.set(nbiny, ymin, ymax);
  _data->resize((_axis_x.getNbins() + 2) * (_axis_y.getNbins() + 2));
}

double Histo2::getBinContent(int nx, int ny) const throw()
{
  return _data->get((nx + 1) + (ny + 1) * (_axis_x.getNbins() + 2));
}

double Histo2::getOverFlow() const throw() { return _data->get(_data->size() - 1); }

double Histo2::getUnderFlow() const throw() { return _data->get(0); }

double Histo2::getOverFlowX(int ny) const throw() { return _data->get(_data->size() - 1); }

double Histo2::getUnderFlowX(int ny) const throw() { return _data->get(0); }

double Histo2::getOverFlowY(int nx) const throw() { return _data->get(_data->size() - 1); }

double Histo2::getUnderFlowY(int nx) const throw() { return _data->get(0); }

void Histo2::setBinContent(int nx, int ny, double data) throw()
{
  _data->set((nx + 1) + (ny + 1) * (_axis_x.getNbins() + 2), data);
}

void Histo2::setOverFlow(double data) throw() { _data->set(_data->size() - 1, data); }

void Histo2::setOverFlowX(int ny, double data) throw() { _data->set(_data->size() - 1, data); }

void Histo2::setOverFlowY(int nx, double data) throw() { _data->set(_data->size() - 1, data); }

void Histo2::setUnderFlow(double data) throw() { _data->set(0, data); }

void Histo2::setUnderFlowX(int ny, double data) throw() { _data->set(0, data); }

void Histo2::setUnderFlowY(int nx, double data) throw() { _data->set(0, data); }

double Histo2::getMaximum() const throw() { return _axis_z.getMax(); }

double Histo2::getMinimum() const throw() { return _axis_z.getMin(); }

void Histo2::setMaximum(double data) throw()
{
  if (!_axis_z.isFixedMax()) _axis_z.setMax(data);
}

void Histo2::setMinimum(double data) throw()
{
  if (!_axis_z.isFixedMin()) _axis_z.setMin(data);
}

void Histo2::fixMaximum(double data) throw()
{
  _axis_z.fixMax(true);
  _axis_z.setMax(data);
}

void Histo2::fixMinimum(double data) throw()
{
  _axis_z.fixMin(true);
  _axis_z.setMin(data);
}

void Histo2::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  Histo::readConfig(reader);
  _axis_x.readObject(reader);
  _axis_y.readObject(reader);
  _data->resize((_axis_x.getNbins() + 2) * (_axis_y.getNbins() + 2));
}

void Histo2::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  Histo::writeConfig(writer);
  _axis_x.writeObject(writer);
  _axis_y.writeObject(writer);
}
