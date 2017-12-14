#include "daq/slc/dqm/MonColor.h"

#include <algorithm>
#include <string>
#include <sstream>

using namespace Belle2;

const MonColor MonColor::NON(-1, -1, -1);
const MonColor MonColor::BLACK(0, 0, 0);
const MonColor MonColor::WHITE(255, 255, 255);
const MonColor MonColor::BLUE(0, 0, 255);
const MonColor MonColor::CYAN(0, 255, 255);
const MonColor MonColor::DARK_GRAY(64, 64, 64);
const MonColor MonColor::GRAY(128, 128, 128);
const MonColor MonColor::GREEN(0, 255, 0);
const MonColor MonColor::LIGHT_GRAY(192, 192, 192);
const MonColor MonColor::MAGENTA(255, 0, 255);
const MonColor MonColor::ORANGE(255, 200, 0);
const MonColor MonColor::PINK(255, 175, 175);
const MonColor MonColor::RED(255, 0, 0);
const MonColor MonColor::YELLOW(255, 255, 0);
const MonColor MonColor::RUNNING_GREEN(0, 204, 102);

bool MonColor::isNon() const throw()
{
  if (m_red < 0 || m_green < 0 || m_blue < 0) {
    return true;
  } else {
    return false;
  }
}

void MonColor::setRGB(int red, int green, int blue) throw()
{
  m_red = (red > 255) ? 255 : red;
  m_green = (green > 255) ? 255 : green;
  m_blue = (blue > 255) ? 255 : blue;
}

const MonColor& MonColor::operator=(const MonColor& c) throw()
{
  setRGB(c.m_red, c.m_green, c.m_blue);
  return *this;
}

std::string MonColor::toString() const throw()
{
  std::stringstream ss;
  if (isNon()) {
    ss << "none";
  } else {
    ss << "#" << std::hex;
    if (m_red > 25) ss  << m_red;
    else if (m_red > 0) ss << "0" << m_red;
    else  ss << "00";
    if (m_green > 25) ss  << m_green;
    else if (m_green > 0) ss << "0" << m_green;
    else  ss << "00";
    if (m_blue > 25) ss  << m_blue;
    else if (m_blue > 0) ss << "0" << m_blue;
    else  ss << "00";
  }
  std::string s = ss.str();
  transform(s.begin(), s.end(), s.begin(), toupper);
  return s;
}
