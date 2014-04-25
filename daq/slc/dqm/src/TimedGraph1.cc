#include "daq/slc/dqm/TimedGraph1.h"

#include "daq/slc/system/Time.h"

#include <sstream>

using namespace Belle2;

TimedGraph1::TimedGraph1()
  : Histo("", ""), m_iter(0), m_time_v(1)
{
  m_axis_x = Axis(1, 0, 1, "");
}

TimedGraph1::TimedGraph1(const std::string& name,
                         const std::string& title,
                         int nbinx, double xmin, double xmax)
  : Histo(name, title), m_iter(0), m_time_v(nbinx)
{
  m_axis_x = Axis(nbinx, xmin, xmax, m_axis_x.getTitle());
}

TimedGraph1::~TimedGraph1() throw()
{
}

void TimedGraph1::reset() throw()
{
  m_iter = 0;
  m_data->setAll(0);
  m_axis_y.setMax(1);
}

void TimedGraph1::setMaximum(double data) throw()
{
  m_axis_y.setMax(data);
}

void TimedGraph1::setMinimum(double data) throw()
{
  m_axis_y.setMin(data);
}

void TimedGraph1::fixMaximum(double data) throw()
{
  m_axis_y.fixMax(true);
  m_axis_y.setMax(data);
}

void TimedGraph1::fixMinimum(double data) throw()
{
  m_axis_y.fixMin(true);
  m_axis_y.setMin(data);
}

double TimedGraph1::getPoint(int n) const throw()
{
  return m_data->get(n);
}

void TimedGraph1::setPoint(int n, double data) throw()
{
  m_data->set(n, data);
}

long TimedGraph1::getTime(int n) const throw()
{
  return (long)m_time_v.get(n);
}

void TimedGraph1::setTime(int n, long long time) throw()
{
  m_time_v.set(n, time);
}

void TimedGraph1::addPoint(long long time, double data) throw()
{
  m_iter++;
  if (m_iter == (int)m_data->size() ||
      m_iter == (int)m_time_v.size()) {
    m_iter = 0;
  }
  setPoint(m_iter, data);
  setTime(m_iter, time);
}

void TimedGraph1::addPoint(double data) throw()
{
  addPoint(Belle2::Time().getSecond(), data);
}

double TimedGraph1::getLatestPoint() const throw()
{
  return m_data->get(m_iter);
  if (m_iter > 0) return m_data->get(m_iter - 1);
  else return m_data->get(m_data->size() - 1);
}

long TimedGraph1::getLatestTime() const throw()
{
  return m_time_v.get(m_iter);
  if (m_iter > 0) return m_time_v.get(m_iter - 1);
  else return m_time_v.get(m_time_v.size() - 1);
}

void TimedGraph1::readObject(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  readConfig(reader);
  m_iter = reader.readInt();
  m_data->readObject(reader);
  m_time_v.readObject(reader);
}

void TimedGraph1::readUpdate(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  long long t = reader.readLong();
  double d = m_data->readElement(reader);
  addPoint(t, d);
}

void TimedGraph1::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  Histo::readConfig(reader);
  m_axis_x.readObject(reader);
  m_axis_y.setTitle(reader.readString());
  m_data->resize(m_axis_x.getNbins());
  m_time_v.resize(m_axis_x.getNbins());
}

void TimedGraph1::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  m_iter = reader.readInt();
  m_time_v.readObject(reader);
  m_data->readObject(reader);
}

void TimedGraph1::writeObject(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writeConfig(writer);
  writer.writeInt(m_iter);
  m_data->writeObject(writer);
  m_time_v.writeObject(writer);
}

void TimedGraph1::writeUpdate(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeLong(getLatestTime());
  m_data->writeElement(getLatestPoint(), writer);
}

void TimedGraph1::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  Histo::writeConfig(writer);
  m_axis_x.writeObject(writer);
  writer.writeString(m_axis_y.getTitle());
}

void TimedGraph1::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeInt(m_iter);
  m_time_v.writeObject(writer);
  m_data->writeObject(writer);
}

std::string TimedGraph1::toString() const throw()
{
  std::stringstream script("");
  script << "<histo data-type='" << getDataType() << "' "
         << "name='" << getName() << "' "
         << "title='" << getTitle() << "' "
         << "tab='" << (int) getTabId() << "' "
         << "position='" << (int) getPositionId() << "' " << std::endl
         << "time='" << m_time_v.toString() << "' " << std::endl
         << "data='" << m_data->toString() << "' >" << std::endl
         << "<axis-x nbins='" << getAxisX().getNbins() << "' "
         << "min='" << getAxisX().getMin() << "' "
         << "max='" << getAxisX().getMax() << "' "
         << "title='" << getAxisX().getTitle() << "' />" << std::endl
         << "<axis-y nbins='" << getAxisY().getNbins() << "' "
         << "min='" << getAxisY().getMin() << "' "
         << "max='" << getAxisY().getMax() << "' "
         << "title='" << getAxisY().getTitle() << "' />" << std::endl
         << "</histo>" << std::endl;

  return script.str();
}

