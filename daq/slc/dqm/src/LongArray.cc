#include "daq/slc/dqm/LongArray.h"

#include <sstream>

using namespace Belle2;

LongArray::LongArray() throw() : NumberArray(0), m_value_v(NULL) {}

LongArray::LongArray(size_t size) throw() : NumberArray(), m_value_v(NULL)
{
  resize(size);
}

LongArray::LongArray(const NumberArray& v) throw() : NumberArray(), m_value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, m_value_v[n]);
}

LongArray::~LongArray() throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
}

double LongArray::get(size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  else return -1;
}
void LongArray::set(size_t n, double v) throw()
{
  if (n < size()) m_value_v[n] = (long long)v;
}
void LongArray::resize(int s) throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
  m_value_v = new long long [s];
  m_size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string LongArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

long long& LongArray::operator[](size_t n) throw()
{
  if (n < size()) return m_value_v[n];
  return m_value_v[0];
}

long long LongArray::operator[](size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  return 0;
}

void LongArray::readObject(Reader& reader)
throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    m_value_v[n] = reader.readLong();
  }
}

void LongArray::writeObject(Writer& writer)
const throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeLong(m_value_v[n]);
  }
}

double LongArray::readElement(Reader& reader)
throw(IOException)
{
  return reader.readLong();
}

void LongArray::writeElement(double value, Writer& writer)
const throw(IOException)
{
  writer.writeLong(value);
}
