#include "daq/slc/dqm/CharArray.h"

#include <sstream>

using namespace Belle2;

CharArray::CharArray() throw() : NumberArray(0), _value_v(NULL) {}

CharArray::CharArray(size_t size) throw() : NumberArray(), _value_v(NULL)
{
  resize(size);
}

CharArray::CharArray(const NumberArray& v) throw() : NumberArray(), _value_v(NULL)
{
  resize(v.size());
  for (size_t n = 0; n < size(); n++) set(n, _value_v[n]);
}

CharArray::~CharArray() throw()
{
  if (_value_v != NULL) delete [] _value_v;
}

double CharArray::get(size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  else return -1;
}
void CharArray::set(size_t n, double v) throw()
{
  if (n < size()) _value_v[n] = (char)v;
}
void CharArray::resize(int s) throw()
{
  if (_value_v != NULL) delete [] _value_v;
  _value_v = new char [s];
  _size = s;
  for (size_t n = 0; n < size(); n++) set(n, 0);
}

std::string CharArray::toScript() const throw()
{
  std::stringstream ss("");
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}

char& CharArray::operator[](size_t n) throw()
{
  if (n < size()) return _value_v[n];
  return _value_v[0];
}

char CharArray::operator[](size_t n) const throw()
{
  if (n < size()) return _value_v[n];
  return 0;
}

void CharArray::readObject(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    _value_v[n] = reader.readChar();
  }
}

void CharArray::writeObject(Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  for (size_t n = 0; n < size(); n++) {
    writer.writeChar(_value_v[n]);
  }
}

double CharArray::readElement(Belle2::Reader& reader)
throw(Belle2::IOException)
{
  return reader.readChar();
}

void CharArray::writeElement(double value, Belle2::Writer& writer)
const throw(Belle2::IOException)
{
  writer.writeChar(value);
}
