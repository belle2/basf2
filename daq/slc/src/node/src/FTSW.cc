#include "FTSW.hh"

#include "FEEModule.hh"

#include <util/StringUtil.hh>

#include <sstream>

using namespace B2DAQ;

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
  ss << Module::getSQLFields() << ", used boolean, channel smallint, firmware text, trigger_mode int";
  for (size_t i = 0; i < _module_v.size(); i++) {
    ss << ", module_type_" << i  << " text, module_id_" << i << " smallint";
  }
  return ss.str();
}

const std::string FTSW::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLLabels() << ", used, channel, firmware, trigger_mode";
  for (size_t i = 0; i < _module_v.size(); i++) {
    ss << ", module_type_" << i  << ", module_id_" << i;
  }
  return ss.str();
}

const std::string FTSW::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLValues()
     << ", " << _used
     << B2DAQ::form(", %d, '%s', ", getChannel(), getFirmware().c_str())
     << _trigger_mode;
  for (size_t i = 0; i < _module_v.size(); i++) {
    if (_module_v[i] != NULL) {
      ss << ", '" << _module_v[i]->getType() << "'"
         << ", " << _module_v[i]->getID();
    } else {
      ss << ", '', 0";
    }
  }
  return ss.str();
}
