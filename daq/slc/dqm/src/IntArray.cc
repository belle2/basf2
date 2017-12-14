#include "daq/slc/dqm/IntArray.h"

#include <sstream>

using namespace Belle2;

IntArray::IntArray() throw() : NumberArray(0), m_value_v(NULL) {}

IntArray::IntArray(size_t size) throw() : NumberArray(), m_value_v(NULL)
{
  resize(size);
}

IntArray::IntArray(const NumberArray& v) throw() : NumberArray(), m_value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, m_value_v[n]);
}

IntArray::~IntArray() throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
}

double IntArray::get(size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  else return -1;
}
void IntArray::set(size_t n, double v) throw()
{
  if (n < size()) m_value_v[n] = (int)v;
}
void IntArray::resize(int s) throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
  m_value_v = new int [s];
  m_size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string IntArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

int& IntArray::operator[](size_t n) throw()
{
  if (n < size()) return m_value_v[n];
  return m_value_v[0];
}

int IntArray::operator[](size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  return 0;
}

void IntArray::readObject(Reader& reader) throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    m_value_v[n] = reader.readInt();
  }
}

void IntArray::writeObject(Writer& writer) const throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeInt(m_value_v[n]);
  }
}

double IntArray::readElement(Reader& reader) throw(IOException)
{
  return reader.readInt();
}

void IntArray::writeElement(double value, Writer& writer)
const throw(IOException)
{
  writer.writeInt(value);
}
