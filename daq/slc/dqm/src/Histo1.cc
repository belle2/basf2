#include "daq/slc/dqm/Histo1.h"

using namespace Belle2;

Histo1::Histo1() throw(): Histo() {}

Histo1::Histo1(const Histo1& h) throw(): Histo(h) {}

Histo1::Histo1(const std::string& name, const std::string& title,
               int nbinx, double min, double max)
throw() : Histo(name, title)
{
  m_axis_x.set(nbinx, min, max);
}

void Histo1::reset() throw()
{
  setUpdateId(0);
  m_data->setAll(0);
}

void Histo1::resetAxis(int nbinx, double xmin, double xmax)
{
  m_axis_x.set(nbinx, xmin, xmax);
  m_data->resize(m_axis_x.getNbins() + 2);
}

double Histo1::getBinContent(int nx) const throw()
{
  return m_data->get(nx + 1);
}

double Histo1::getOverFlow() const throw()
{
  return m_data->get(m_axis_x.getNbins() + 1);
}

double Histo1::getUnderFlow() const throw()
{
  return m_data->get(0);
}

void Histo1::setBinContent(int nx, double data) throw()
{
  m_data->set(nx + 1, data);
}

void Histo1::setOverFlow(double data) throw()
{
  m_data->set(m_axis_x.getNbins() + 1, data);
}

void Histo1::setUnderFlow(double data) throw()
{
  m_data->set(0, data);
}

double Histo1::getMaximum() const throw()
{
  return m_axis_y.getMax();
}

double Histo1::getMinimum() const throw()
{
  return m_axis_y.getMin();
}

void Histo1::setMaximum(double data) throw()
{
  if (!m_axis_y.isFixedMax()) m_axis_y.setMax(data);
}

void Histo1::setMinimum(double data) throw()
{
  if (!m_axis_y.isFixedMin()) m_axis_y.setMin(data);
}

void Histo1::fixMaximum(double data) throw()
{
  m_axis_y.fixMax(true);
  m_axis_y.setMax(data);
}

void Histo1::fixMinimum(double data) throw()
{
  m_axis_y.fixMin(true);
  m_axis_y.setMin(data);
}

void Histo1::readConfig(Reader& reader) throw(IOException)
{
  Histo::readConfig(reader);
  m_axis_x.readObject(reader);
  m_axis_y.setTitle(reader.readString());
  m_data->resize(m_axis_x.getNbins() + 2);
}

void Histo1::writeConfig(Writer& writer) const throw(IOException)
{
  Histo::writeConfig(writer);
  m_axis_x.writeObject(writer);
  writer.writeString(m_axis_y.getTitle());
}
