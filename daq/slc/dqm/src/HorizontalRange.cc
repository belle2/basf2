#include "daq/slc/dqm/HorizontalRange.h"

using namespace Belle2;

HorizontalRange::HorizontalRange() : Range()
{

}

HorizontalRange::HorizontalRange(const std::string& name) : Range(name)
{

}

HorizontalRange::HorizontalRange(const std::string& name, double min, double max)
  : Range(name, min, max)
{

}

HorizontalRange::~HorizontalRange() throw()
{

}

std::string HorizontalRange::getDataType() const throw()
{
  return "R1H";
}
