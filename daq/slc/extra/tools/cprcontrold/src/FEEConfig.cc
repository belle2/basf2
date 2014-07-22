#include "FEEConfig.h"

using namespace Belle2;

void FEEConfig::Register::read(const ConfigObject& obj)
{
  m_name = obj.getText("paramname");
  m_address = obj.getInt("address");
  m_size = obj.getInt("size");
}

void FEEConfig::Parameter::read(const ConfigObject& obj)
{
  m_name = obj.getText("paramname");
  m_index = obj.getInt("arrayindex");
  m_value = obj.getInt("value");
}

void FEEConfig::read(const ConfigObject& obj)
{
  clear();
  m_slot = obj.getInt("slot");
  m_board = obj.getText("board");
  m_firmware = obj.getText("firmware");
  const ConfigObjectList& registers(obj.getObjects("register"));
  for (size_t i = 0; i < registers.size(); i++) {
    Register reg;
    reg.read(registers[i]);
    m_register.push_back(reg);
  }
  const ConfigObjectList& parameters(obj.getObjects("parameter"));
  for (size_t i = 0; i < parameters.size(); i++) {
    Parameter param;
    param.read(parameters[i]);
    m_parameter.push_back(param);
  }
}
