#include "FTSW.h"

#include "FEEModule.h"

#include "StringUtil.h"

#include <sstream>

using namespace Belle2;

const int FTSW::TRIG_NORMAL = 0;
const int FTSW::TRIG_IN = 1;
const int FTSW::TRIG_TLU = 2;
const int FTSW::TRIG_PULSE = 4;
const int FTSW::TRIG_REVO = 5;
const int FTSW::TRIG_RANDOM = 6;
const int FTSW::TRIG_POSSION = 7;
const int FTSW::TRIG_ONCE = 8;
const int FTSW::TRIG_STOP = 9;

const std::string FTSW::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLFields()
     << ", used boolean, channel smallint, firmware text"
     << ", trigger_mode int, dummy_rate int, trigger_limit int";
  return ss.str();
}

const std::string FTSW::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLLabels()
     << ", used, channel, firmware"
     << ", trigger_mode, dummy_rate, trigger_limit";
  return ss.str();
}

const std::string FTSW::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLValues()
     << ", " << _used << ", "
     << getChannel() << ", '" << getFirmware().c_str() << "', "
     << _trigger_mode << ", " << _dummy_rate << ", " << _trigger_limit;
  return ss.str();
}
