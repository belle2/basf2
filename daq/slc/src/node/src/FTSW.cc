#include "FTSW.hh"

#include "FEEModule.hh"

#include <util/StringUtil.hh>

#include <sstream>

using namespace B2DAQ;

const std::string FTSW::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLFields() << ", channel smallint, firmware text";
  for (size_t i = 0; i < _module_v.size(); i++) {
    ss << ", module_type_" << i  << " text, module_id_" << i << " smallint";
  }
  return ss.str();
}

const std::string FTSW::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLLabels() << ", channel, firmware";
  for (size_t i = 0; i < _module_v.size(); i++) {
    ss << ", module_type_" << i  << ", module_id_" << i;
  }
  return ss.str();
}

const std::string FTSW::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLValues()
     << B2DAQ::form(", %d, '%s'", getChannel(), getFirmware().c_str());
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
