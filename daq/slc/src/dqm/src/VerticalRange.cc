#include "VerticalRange.hh"

using namespace B2DQM;

VerticalRange::VerticalRange() : Range()
{

}

VerticalRange::VerticalRange(const std::string& name) : Range(name)
{

}

VerticalRange::VerticalRange(const std::string& name, double min, double max)
  : Range(name, min, max)
{

}

VerticalRange::~VerticalRange() throw()
{

}

std::string VerticalRange::getDataType() const throw()
{
  return "R1V";
}
