#include "daq/slc/dqm/Histo2.h"

using namespace Belle2;

Histo2::Histo2() throw() : Histo() {}

Histo2::Histo2(const Histo2& h) throw() : Histo(h) {}

Histo2::Histo2(const std::string& name, const std::string& title,
               int nbinx, double xmin, double xmax,
               int nbiny, double ymin, double ymax) throw() : Histo(name, title)
{
  m_axis_x.set(nbinx, xmin, xmax);
  m_axis_y.set(nbiny, ymin, ymax);
}

void Histo2::reset() throw()
{
  setUpdateId(0);
  m_data->setAll(0);
}

void Histo2::resetAxis(int nbinx, double xmin, double xmax,
                       int nbiny, double ymin, double ymax)
{
  m_axis_x.set(nbinx, xmin, xmax);
  m_axis_y.set(nbiny, ymin, ymax);
  m_data->resize((m_axis_x.getNbins() + 2) * (m_axis_y.getNbins() + 2));
}

double Histo2::getBinContent(int nx, int ny) const throw()
{
  return m_data->get((nx + 1) + (ny + 1) * (m_axis_x.getNbins() + 2));
}

double Histo2::getOverFlow() const throw() { return m_data->get(m_data->size() - 1); }

double Histo2::getUnderFlow() const throw() { return m_data->get(0); }

double Histo2::getOverFlowX(int) const throw() { return m_data->get(m_data->size() - 1); }

double Histo2::getUnderFlowX(int) const throw() { return m_data->get(0); }

double Histo2::getOverFlowY(int) const throw() { return m_data->get(m_data->size() - 1); }

double Histo2::getUnderFlowY(int) const throw() { return m_data->get(0); }

void Histo2::setBinContent(int nx, int ny, double data) throw()
{
  m_data->set((nx + 1) + (ny + 1) * (m_axis_x.getNbins() + 2), data);
}

void Histo2::setOverFlow(double data) throw() { m_data->set(m_data->size() - 1, data); }

void Histo2::setOverFlowX(int, double data) throw() { m_data->set(m_data->size() - 1, data); }

void Histo2::setOverFlowY(int, double data) throw() { m_data->set(m_data->size() - 1, data); }

void Histo2::setUnderFlow(double data) throw() { m_data->set(0, data); }

void Histo2::setUnderFlowX(int, double data) throw() { m_data->set(0, data); }

void Histo2::setUnderFlowY(int, double data) throw() { m_data->set(0, data); }

double Histo2::getMaximum() const throw() { return m_axis_z.getMax(); }

double Histo2::getMinimum() const throw() { return m_axis_z.getMin(); }

void Histo2::setMaximum(double data) throw()
{
  if (!m_axis_z.isFixedMax()) m_axis_z.setMax(data);
}

void Histo2::setMinimum(double data) throw()
{
  if (!m_axis_z.isFixedMin()) m_axis_z.setMin(data);
}

void Histo2::fixMaximum(double data) throw()
{
  m_axis_z.fixMax(true);
  m_axis_z.setMax(data);
}

void Histo2::fixMinimum(double data) throw()
{
  m_axis_z.fixMin(true);
  m_axis_z.setMin(data);
}

void Histo2::readConfig(Reader& reader) throw(IOException)
{
  Histo::readConfig(reader);
  m_axis_x.readObject(reader);
  m_axis_y.readObject(reader);
  m_data->resize((m_axis_x.getNbins() + 2) * (m_axis_y.getNbins() + 2));
}

void Histo2::writeConfig(Writer& writer) const throw(IOException)
{
  Histo::writeConfig(writer);
  m_axis_x.writeObject(writer);
  m_axis_y.writeObject(writer);
}
