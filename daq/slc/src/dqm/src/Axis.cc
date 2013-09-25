#include "Axis.hh"

#include <sstream>

using namespace B2DQM;

Axis::Axis()
  : _nbins(1), _min(0), _max(1), _title(""),
    _fixed_min(false), _fixed_max(false) {}

Axis::Axis(int nbin, double min, double max,
           const std::string& title)
  : _nbins(nbin), _min(min), _max(max), _title(title),
    _fixed_min(false), _fixed_max(false) {}

Axis::Axis(int nbin, double min, double max)
  : _nbins(nbin), _min(min), _max(max),
    _title(""), _fixed_min(false), _fixed_max(false) {}

Axis::Axis(const Axis& axis)
  : _nbins(axis._nbins), _min(axis._min), _max(axis._max),
    _title(axis._title), _fixed_min(false), _fixed_max(false) {}

Axis::~Axis() throw() {}

void Axis::set(int nbin, double min, double max)
{
  setNbins(nbin);
  setMin(min);
  setMax(max);
}

void Axis::set(int nbin, double min, double max, const std::string& title)
{
  set(nbin, min, max);
  setTitle(title);
}

void Axis::readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  _title = reader.readString();
  setNbins(reader.readInt());
  setMin(reader.readDouble());
  setMax(reader.readDouble());
}

void Axis::writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException)
{
  writer.writeString(_title);
  writer.writeInt(getNbins());
  writer.writeDouble(getMin());
  writer.writeDouble(getMax());
}

const Axis& Axis::operator=(const Axis& axis)
{
  _nbins = axis._nbins;
  _min = axis._min;
  _max = axis._max;
  _title = axis._title;
  return *this;
}
