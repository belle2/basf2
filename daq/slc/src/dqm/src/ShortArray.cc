#include "ShortArray.hh"

#include <sstream>

using namespace B2DQM;

ShortArray::ShortArray() throw() : NumberArray(0), _value_v(NULL) {}

ShortArray::ShortArray(size_t size) throw() : NumberArray(), _value_v(NULL)
{
  resize(size);
}

ShortArray::ShortArray(const NumberArray& v) throw() : NumberArray(), _value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, _value_v[n]);
}

ShortArray::~ShortArray() throw()
{
  if (_value_v != NULL) delete [] _value_v;
}

double ShortArray::get(size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  else return -1;
}
void ShortArray::set(size_t n, double v) throw()
{
  if (n < size()) _value_v[n] = (short)v;
}
void ShortArray::resize(int s) throw()
{
  if (_value_v != NULL) delete [] _value_v;
  _value_v = new short [s];
  _size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string ShortArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

short& ShortArray::operator[](size_t n) throw()
{
  if (n < size()) return _value_v[n];
  return _value_v[0];
}

short ShortArray::operator[](size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  return 0;
}

void ShortArray::readObject(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    _value_v[n] = reader.readShort();
  }
}

void ShortArray::writeObject(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeShort(_value_v[n]);
  }
}

double ShortArray::readElement(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  return reader.readShort();
}

void ShortArray::writeElement(double value, B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  writer.writeShort(value);
}
