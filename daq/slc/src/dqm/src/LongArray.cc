#include "LongArray.h"

#include <sstream>

using namespace Belle2;

LongArray::LongArray() throw() : NumberArray(0), _value_v(NULL) {}

LongArray::LongArray(size_t size) throw() : NumberArray(), _value_v(NULL)
{
  resize(size);
}

LongArray::LongArray(const NumberArray& v) throw() : NumberArray(), _value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, _value_v[n]);
}

LongArray::~LongArray() throw()
{
  if (_value_v != NULL) delete [] _value_v;
}

double LongArray::get(size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  else return -1;
}
void LongArray::set(size_t n, double v) throw()
{
  if (n < size()) _value_v[n] = (long long)v;
}
void LongArray::resize(int s) throw()
{
  if (_value_v != NULL) delete [] _value_v;
  _value_v = new long long [s];
  _size = s;
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
  if (n < size()) return _value_v[n];
  return _value_v[0];
}

long long LongArray::operator[](size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  return 0;
}

void LongArray::readObject(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    _value_v[n] = reader.readLong();
  }
}

void LongArray::writeObject(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeLong(_value_v[n]);
  }
}

double LongArray::readElement(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  return reader.readLong();
}

void LongArray::writeElement(double value, Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeLong(value);
}
