#include "HSLB.h"

#include "FEEModule.h"

#include "StringUtil.h"

using namespace Belle2;

const std::string HSLB::getSQLFields() const throw()
{
  return Module::getSQLFields() +
         ", used boolean, firmware text, trigger_mode smallint, module_type text, module_id int";
}

const std::string HSLB::getSQLLabels() const throw()
{
  return Module::getSQLLabels() +
         ", used, firmware, trigger_mode, module_type, module_id";
}

const std::string HSLB::getSQLValues() const throw()
{
  return Module::getSQLValues() +
         Belle2::form(", %s, '%s', %d, '%s', %d",
                      ((isUsed()) ? "TRUE" : "FALSE"),
                      getFirmware().c_str(),
                      getTriggerMode(),
                      getFEEModule()->getType().c_str(),
                      getFEEModule()->getID());
}
