#include "daq/slc/dqm/Graph1.h"

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
  m_axis_x.set(nbinx, xmin, xmax);
  m_axis_y.set(1, ymin, ymax);
}

Graph1::~Graph1() throw() {}

double Graph1::getPointX(int n) const throw()
{
  return m_data->get(n);
}

double Graph1::getPointY(int n) const throw()
{
  return m_data->get(m_data->size() / 2 + n);
}

void Graph1::setPointX(int n, double data) throw()
{
  m_data->set(n, data);
}

void Graph1::setPointY(int n, double data) throw()
{
  m_data->set(m_data->size() / 2 + n, data);
}

double Graph1::getMaximum() const throw()
{
  return m_axis_y.getMax();
}

double Graph1::getMinimum() const throw()
{
  return m_axis_y.getMin();
}

void Graph1::setMaximum(double data) throw()
{
  if (m_axis_y.isFixedMax()) m_axis_y.setMax(data);
}

void Graph1::setMinimum(double data) throw()
{
  if (m_axis_y.isFixedMin()) m_axis_y.setMin(data);
}

void Graph1::fixMaximum(double data) throw()
{
  m_axis_y.fixMax(true);
  m_axis_y.setMax(data);
}

void Graph1::fixMinimum(double data) throw()
{
  m_axis_y.fixMin(true);
  m_axis_y.setMin(data);
}

void Graph1::readConfig(Reader& reader) throw(IOException)
{
  Histo::readConfig(reader);
  m_axis_x.readObject(reader);
  m_axis_y.readObject(reader);
  m_data->resize(m_axis_x.getNbins() * 2);
}

void Graph1::writeConfig(Writer& writer) const throw(IOException)
{
  Histo::writeConfig(writer);
  m_axis_x.writeObject(writer);
  m_axis_y.writeObject(writer);
}

void Graph1::reset() throw()
{
  setUpdateId(0);
  m_data->setAll(0);
}
