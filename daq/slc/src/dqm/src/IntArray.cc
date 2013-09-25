#include "IntArray.hh"

#include <sstream>

using namespace B2DQM;

IntArray::IntArray() throw() : NumberArray(0), _value_v(NULL) {}

IntArray::IntArray(size_t size) throw() : NumberArray(), _value_v(NULL)
{
  resize(size);
}

IntArray::IntArray(const NumberArray& v) throw() : NumberArray(), _value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, _value_v[n]);
}

IntArray::~IntArray() throw()
{
  if (_value_v != NULL) delete [] _value_v;
}

double IntArray::get(size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  else return -1;
}
void IntArray::set(size_t n, double v) throw()
{
  if (n < size()) _value_v[n] = (int)v;
}
void IntArray::resize(int s) throw()
{
  if (_value_v != NULL) delete [] _value_v;
  _value_v = new int [s];
  _size = s;
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
  if (n < size()) return _value_v[n];
  return _value_v[0];
}

int IntArray::operator[](size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  return 0;
}

void IntArray::readObject(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    _value_v[n] = reader.readInt();
  }
}

void IntArray::writeObject(B2DAQ::Writer& writer) const throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeInt(_value_v[n]);
  }
}

double IntArray::readElement(B2DAQ::Reader& reader) throw(B2DAQ::IOException)
{
  return reader.readInt();
}

void IntArray::writeElement(double value, B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  writer.writeInt(value);
}
