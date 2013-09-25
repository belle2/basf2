#include "HSLB.hh"

#include "FEEModule.hh"

#include <util/StringUtil.hh>

using namespace B2DAQ;

const std::string HSLB::getSQLFields() const throw()
{
  return Module::getSQLFields() +
         ", used boolean, firmware text, module_type text, module_id int";
}

const std::string HSLB::getSQLLabels() const throw()
{
  return Module::getSQLLabels() +
         ", used, firmware, module_type, module_id";
}

const std::string HSLB::getSQLValues() const throw()
{
  return Module::getSQLValues() +
         B2DAQ::form(", %s, '%s', '%s', %d",
                     ((isUsed()) ? "TRUE" : "FALSE"),
                     getFirmware().c_str(),
                     getFEEModule()->getType().c_str(),
                     getFEEModule()->getID());
}
