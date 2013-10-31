#include "MonColor.h"

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
  if (_red < 0 || _green < 0 || _blue < 0) {
    return true;
  } else {
    return false;
  }
}

void MonColor::setRGB(int red, int green, int blue) throw()
{
  _red = (red > 255) ? 255 : red;
  _green = (green > 255) ? 255 : green;
  _blue = (blue > 255) ? 255 : blue;
}

const MonColor& MonColor::operator=(const MonColor& c) throw()
{
  setRGB(c._red, c._green, c._blue);
  return *this;
}

std::string MonColor::toString() const throw()
{
  std::stringstream ss;
  if (isNon()) {
    ss << "none";
  } else {
    ss << "#" << std::hex;
    if (_red > 25) ss  << _red;
    else if (_red > 0) ss << "0" << _red;
    else  ss << "00";
    if (_green > 25) ss  << _green;
    else if (_green > 0) ss << "0" << _green;
    else  ss << "00";
    if (_blue > 25) ss  << _blue;
    else if (_blue > 0) ss << "0" << _blue;
    else  ss << "00";
  }
  std::string s = ss.str();
  transform(s.begin(), s.end(), s.begin(), toupper);
  return s;
}
