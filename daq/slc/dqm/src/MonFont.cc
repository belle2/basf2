#include "daq/slc/dqm/MonFont.h"

using namespace Belle2;

MonFont::MonFont(const MonColor& color, int size,
                 const std::string& family) throw()
  : _color(color), _size(size), _family(family) {}

const MonFont& MonFont::operator=(const MonFont& font) throw()
{
  _color = font._color;
  _size = font._size;
  _family = font._family;
  return *this;
}
