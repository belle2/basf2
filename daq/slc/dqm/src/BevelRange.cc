#include "daq/slc/dqm/BevelRange.h"

using namespace Belle2;

BevelRange::BevelRange() : Range()
{

}

BevelRange::BevelRange(const std::string& name) : Range(name, 1, 0)
{

}

BevelRange::BevelRange(const std::string& name, double a, double b)
  : Range(name, a, b)  // y = a*x + b
{
}

BevelRange::~BevelRange() throw()
{

}

std::string BevelRange::getDataType() const throw()
{
  return "R1B";
}

