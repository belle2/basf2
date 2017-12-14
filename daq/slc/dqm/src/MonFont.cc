#include "daq/slc/dqm/MonFont.h"

using namespace Belle2;

MonFont::MonFont(const MonColor& color, int size,
                 const std::string& family) throw()
  : m_color(color), m_size(size), m_family(family) {}

const MonFont& MonFont::operator=(const MonFont& font) throw()
{
  m_color = font.m_color;
  m_size = font.m_size;
  m_family = font.m_family;
  return *this;
}
