#include "daq/slc/dqm/TimedGraph1.h"

#include "daq/slc/system/Time.h"

#include <sstream>

using namespace Belle2;

TimedGraph1::TimedGraph1()
  : Histo("", ""), _iter(0), _time_v(1)
{
  _axis_x = Axis(1, 0, 1, "");
}

TimedGraph1::TimedGraph1(const std::string& name,
                         const std::string& title,
                         int nbinx, double xmin, double xmax)
  : Histo(name, title), _iter(0), _time_v(nbinx)
{
  _axis_x = Axis(nbinx, xmin, xmax, _axis_x.getTitle());
}

TimedGraph1::~TimedGraph1() throw()
{
}

void TimedGraph1::reset() throw()
{
  _iter = 0;
  _data->setAll(0);
  _axis_y.setMax(1);
}

void TimedGraph1::setMaximum(double data) throw()
{
  _axis_y.setMax(data);
}

void TimedGraph1::setMinimum(double data) throw()
{
  _axis_y.setMin(data);
}

void TimedGraph1::fixMaximum(double data) throw()
{
  _axis_y.fixMax(true);
  _axis_y.setMax(data);
}

void TimedGraph1::fixMinimum(double data) throw()
{
  _axis_y.fixMin(true);
  _axis_y.setMin(data);
}

double TimedGraph1::getPoint(int n) const throw()
{
  return _data->get(n);
}

void TimedGraph1::setPoint(int n, double data) throw()
{
  _data->set(n, data);
}

long TimedGraph1::getTime(int n) const throw()
{
  return (long)_time_v.get(n);
}

void TimedGraph1::setTime(int n, long long time) throw()
{
  _time_v.set(n, time);
}

void TimedGraph1::addPoint(long long time, double data) throw()
{
  _iter++;
  if (_iter == (int)_data->size() ||
      _iter == (int)_time_v.size()) {
    _iter = 0;
  }
  setPoint(_iter, data);
  setTime(_iter, time);
}

void TimedGraph1::addPoint(double data) throw()
{
  addPoint(Belle2::Time().getSecond(), data);
}

double TimedGraph1::getLatestPoint() const throw()
{
  return _data->get(_iter);
  if (_iter > 0) return _data->get(_iter - 1);
  else return _data->get(_data->size() - 1);
}

long TimedGraph1::getLatestTime() const throw()
{
  return _time_v.get(_iter);
  if (_iter > 0) return _time_v.get(_iter - 1);
  else return _time_v.get(_time_v.size() - 1);
}

void TimedGraph1::readObject(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  readConfig(reader);
  _iter = reader.readInt();
  _data->readObject(reader);
  _time_v.readObject(reader);
}

void TimedGraph1::readUpdate(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  long long t = reader.readLong();
  double d = _data->readElement(reader);
  addPoint(t, d);
}

void TimedGraph1::readConfig(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  Histo::readConfig(reader);
  _axis_x.readObject(reader);
  _axis_y.setTitle(reader.readString());
  _data->resize(_axis_x.getNbins());
  _time_v.resize(_axis_x.getNbins());
}

void TimedGraph1::readContents(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  _iter = reader.readInt();
  _time_v.readObject(reader);
  _data->readObject(reader);
}

void TimedGraph1::writeObject(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writeConfig(writer);
  writer.writeInt(_iter);
  _data->writeObject(writer);
  _time_v.writeObject(writer);
}

void TimedGraph1::writeUpdate(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeLong(getLatestTime());
  _data->writeElement(getLatestPoint(), writer);
}

void TimedGraph1::writeConfig(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  Histo::writeConfig(writer);
  _axis_x.writeObject(writer);
  writer.writeString(_axis_y.getTitle());
}

void TimedGraph1::writeContents(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeInt(_iter);
  _time_v.writeObject(writer);
  _data->writeObject(writer);
}

std::string TimedGraph1::toString() const throw()
{
  std::stringstream script("");
  script << "<histo data-type='" << getDataType() << "' "
         << "name='" << getName() << "' "
         << "title='" << getTitle() << "' "
         << "tab='" << (int) getTabId() << "' "
         << "position='" << (int) getPositionId() << "' " << std::endl
         << "time='" << _time_v.toString() << "' " << std::endl
         << "data='" << _data->toString() << "' >" << std::endl
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

