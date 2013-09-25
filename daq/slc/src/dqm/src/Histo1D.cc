#include "Histo1D.hh"

#include "DoubleArray.hh"

using namespace B2DQM;

Histo1D::Histo1D() throw() : Histo1()
{
  _data = new DoubleArray(0);
}

Histo1D::Histo1D(const Histo1D& h) throw() : Histo1(h)
{
  _data = new DoubleArray(h._data->size());
}

Histo1D::Histo1D(const std::string& name, const std::string& title, int nbinx, double min, double max)
throw() : Histo1(name, title, nbinx, min, max)
{
  _data = new DoubleArray(nbinx + 2);
}

Histo1D::~Histo1D() throw() {}

std::string Histo1D::getDataType() const throw()
{
  return std::string("H1D");
}
