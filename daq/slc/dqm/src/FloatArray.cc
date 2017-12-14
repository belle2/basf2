#include "daq/slc/dqm/FloatArray.h"

#include <sstream>

using namespace Belle2;

FloatArray::FloatArray() throw() : NumberArray(0), m_value_v(NULL) {}

FloatArray::FloatArray(size_t size) throw() : NumberArray(), m_value_v(NULL)
{
  resize(size);
}

FloatArray::FloatArray(const NumberArray& v) throw() : NumberArray(), m_value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, m_value_v[n]);
}

FloatArray::~FloatArray() throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
}

double FloatArray::get(size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  else return -1;
}
void FloatArray::set(size_t n, double v) throw()
{
  if (n < size()) m_value_v[n] = (float)v;
}
void FloatArray::resize(int s) throw()
{
  if (m_value_v != NULL) delete [] m_value_v;
  m_value_v = new float [s];
  m_size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string FloatArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

float& FloatArray::operator[](size_t n) throw()
{
  if (n < size()) return m_value_v[n];
  return m_value_v[0];
}

float FloatArray::operator[](size_t n) const throw()
{
  if (n < size()) return m_value_v[n];
  return 0;
}

void FloatArray::readObject(Reader& reader) throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    m_value_v[n] = reader.readFloat();
  }
}

double FloatArray::readElement(Reader& reader)
throw(IOException)
{
  return reader.readFloat();
}

void FloatArray::writeObject(Writer& writer)
const throw(IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeFloat(m_value_v[n]);
  }
}

void FloatArray::writeElement(double value, Writer& writer)
const throw(IOException)
{
  writer.writeFloat(value);
}


