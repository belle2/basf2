#include "daq/slc/apps/cprcontrold/FEEConfig.h"

using namespace Belle2;

void FEEConfig::Register::read(const DBObject& obj)
{
  m_name = obj.getText("paramname");
  m_address = obj.getInt("address");
  m_size = obj.getInt("size");
}

void FEEConfig::Parameter::read(const DBObject& obj)
{
  m_name = obj.getText("paramname");
  m_index = obj.getInt("arrayindex");
  m_value = obj.getInt("value");
}

void FEEConfig::read(const DBObject& obj)
{
  clear();
  m_slot = obj.getInt("slot");
  m_board = obj.getText("board");
  m_firmware = obj.getText("firmware");
  const DBObjectList& registers(obj.getObjects("register"));
  for (size_t i = 0; i < registers.size(); i++) {
    Register reg;
    reg.read(registers[i]);
    m_register.push_back(reg);
  }
  const DBObjectList& parameters(obj.getObjects("parameter"));
  for (size_t i = 0; i < parameters.size(); i++) {
    Parameter param;
    param.read(parameters[i]);
    m_parameter.push_back(param);
  }
}

FEEConfig::RegisterList::iterator
FEEConfig::getRegister(const std::string& name)
{
  for (FEEConfig::RegisterList::iterator it = m_register.begin();
       it != m_register.end(); it++) {
    if (it->getParamName() == name) return it;
  }
  return m_register.end();
}
