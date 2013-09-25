#include "DoubleArray.hh"

#include <sstream>

using namespace B2DQM;

DoubleArray::DoubleArray() throw()
  : NumberArray(0), _value_v(NULL) {}

DoubleArray::DoubleArray(size_t size) throw()
  : NumberArray(), _value_v(NULL)
{
  resize(size);
}

DoubleArray::DoubleArray(const NumberArray& v) throw()
  : NumberArray(), _value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, _value_v[n]);
}

DoubleArray::~DoubleArray() throw()
{
  if (_value_v != NULL) delete [] _value_v;
}

double DoubleArray::get(size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  else return -1;
}
void DoubleArray::set(size_t n, double v) throw()
{
  if (n < size()) _value_v[n] = (double)v;
}
void DoubleArray::resize(int s) throw()
{
  if (_value_v != NULL) delete [] _value_v;
  _value_v = new double [s];
  _size = s;
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
  if (n < size()) return _value_v[n];
  return _value_v[0];
}

double DoubleArray::operator[](size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  return 0;
}

void DoubleArray::readObject(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    _value_v[n] = reader.readDouble();
  }
}

void DoubleArray::writeObject(B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeDouble(_value_v[n]);
  }
}

double DoubleArray::readElement(B2DAQ::Reader& reader)
throw(B2DAQ::IOException)
{
  return reader.readDouble();
}

void DoubleArray::writeElement(double value, B2DAQ::Writer& writer)
const throw(B2DAQ::IOException)
{
  writer.writeDouble(value);
}


