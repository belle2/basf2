#include "daq/slc/dqm/NumberArray.h"

#include <sstream>

using namespace Belle2;

NumberArray::NumberArray() throw()
  : m_size(0) {}

NumberArray::NumberArray(size_t len) throw()
  : m_size(len) {}

NumberArray::NumberArray(const NumberArray& v) throw()
  : m_size(v.m_size) {}

NumberArray::~NumberArray() throw() {}

void NumberArray::setAll(const double v) throw()
{
  for (size_t n = 0; n < size(); n++) {
    set(n, v);
  }
}

const NumberArray& NumberArray::operator=(const NumberArray& v) throw()
{
  if (size() != v.size()) resize(v.size());
  for (size_t n = 0; n < size(); n++) {
    set(n, v.get(n));
  }
  return *this;
}
void NumberArray::copy(const NumberArray& v) throw()
{
  if (size() != v.size()) resize(v.size());
  for (size_t n = 0; n < size(); n++) {
    set(n, v.get(n));
  }
}

std::string NumberArray::toString() const throw()
{
  std::stringstream ss;
  for (size_t n = 0; n < size(); n++) {
    ss << get(n);
    if (n < size() - 1) ss << ",";
  }
  return ss.str();
}
