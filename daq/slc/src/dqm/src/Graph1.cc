#include "dqm/Graph1.h"

using namespace Belle2;

Graph1::Graph1() throw()
  : Histo() {}

Graph1::Graph1(const Graph1& h) throw()
  : Histo(h) {}

Graph1::Graph1(std::string name, std::string title,
               int nbinx, double xmin, double xmax,
               double ymin, double ymax) throw()
  : Histo(name, title)
{
  _axis_x.set(nbinx, xmin, xmax);
  _axis_y.set(1, ymin, ymax);
}

Graph1::~Graph1() throw() {}

double Graph1::getPointX(int n) const throw()
{
  return _data->get(n);
}

double Graph1::getPointY(int n) const throw()
{
  return _data->get(_data->size() / 2 + n);
}

void Graph1::setPointX(int n, double data) throw()
{
  _data->set(n, data);
}

void Graph1::setPointY(int n, double data) throw()
{
  _data->set(_data->size() / 2 + n, data);
}

double Graph1::getMaximum() const throw()
{
  return _axis_y.getMax();
}

double Graph1::getMinimum() const throw()
{
  return _axis_y.getMin();
}

void Graph1::setMaximum(double data) throw()
{
  if (_axis_y.isFixedMax()) _axis_y.setMax(data);
}

void Graph1::setMinimum(double data) throw()
{
  if (_axis_y.isFixedMin()) _axis_y.setMin(data);
}

void Graph1::fixMaximum(double data) throw()
{
  _axis_y.fixMax(true);
  _axis_y.setMax(data);
}

void Graph1::fixMinimum(double data) throw()
{
  _axis_y.fixMin(true);
  _axis_y.setMin(data);
}

void Graph1::readConfig(Belle2::Reader& reader) throw(Belle2::IOException)
{
  Histo::readConfig(reader);
  _axis_x.readObject(reader);
  _axis_y.readObject(reader);
  _data->resize(_axis_x.getNbins() * 2);
}

void Graph1::writeConfig(Belle2::Writer& writer) const throw(Belle2::IOException)
{
  Histo::writeConfig(writer);
  _axis_x.writeObject(writer);
  _axis_y.writeObject(writer);
}

void Graph1::reset() throw()
{
  setUpdateId(0);
  _data->setAll(0);
}
