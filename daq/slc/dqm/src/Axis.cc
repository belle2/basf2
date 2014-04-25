#include "daq/slc/dqm/Axis.h"

#include <sstream>

using namespace Belle2;

Axis::Axis()
  : m_nbins(1), m_min(0), m_max(1), m_title(""),
    m_fixed_min(false), m_fixed_max(false) {}

Axis::Axis(int nbin, double min, double max,
           const std::string& title)
  : m_nbins(nbin), m_min(min), m_max(max), m_title(title),
    m_fixed_min(false), m_fixed_max(false) {}

Axis::Axis(int nbin, double min, double max)
  : m_nbins(nbin), m_min(min), m_max(max),
    m_title(""), m_fixed_min(false), m_fixed_max(false) {}

Axis::Axis(const Axis& axis)
  : m_nbins(axis.m_nbins), m_min(axis.m_min), m_max(axis.m_max),
    m_title(axis.m_title), m_fixed_min(false), m_fixed_max(false) {}

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

void Axis::readObject(Reader& reader) throw(IOException)
{
  m_title = reader.readString();
  setNbins(reader.readInt());
  setMin(reader.readDouble());
  setMax(reader.readDouble());
}

void Axis::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(m_title);
  writer.writeInt(getNbins());
  writer.writeDouble(getMin());
  writer.writeDouble(getMax());
}

const Axis& Axis::operator=(const Axis& axis)
{
  m_nbins = axis.m_nbins;
  m_min = axis.m_min;
  m_max = axis.m_max;
  m_title = axis.m_title;
  return *this;
}
