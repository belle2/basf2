#include "daq/slc/dqm/DoubleArray.h"

#include <sstream>

using namespace Belle2;

DoubleArray::DoubleArray() throw()
  : NumberArray(0), m_value_v(NULL) {}

DoubleArray::DoubleArray(size_t size) throw()
  : NumberArray(), m_value_v(NULL)
{
  resize(size);
}

DoubleArray::DoubleArray(const NumberArray& v) throw()
  : NumberArray(), m_value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, m_value_v[n]);
}

DoubleArray::~DoubleArray() throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
}

double DoubleArray::get(size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  else return -1;
}
void DoubleArray::set(size_t n, double v) throw()
{
  if (n < size()) m_value_v[n] = (double)v;
}
void DoubleArray::resize(int s) throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
  m_value_v = new double [s];
  m_size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string DoubleArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

double& DoubleArray::operator[](size_t n) throw()
{
  if (n < size()) return m_value_v[n];
  return m_value_v[0];
}

double DoubleArray::operator[](size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  return 0;
}

void DoubleArray::readObject(Reader& reader)
throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    m_value_v[n] = reader.readDouble();
  }
}

void DoubleArray::writeObject(Writer& writer)
const throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeDouble(m_value_v[n]);
  }
}

double DoubleArray::readElement(Reader& reader)
throw(IOException)
{
  return reader.readDouble();
}

void DoubleArray::writeElement(double value, Writer& writer)
const throw(IOException)
{
  writer.writeDouble(value);
}


