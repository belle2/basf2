#include "FEEModule.h"

#include "StringUtil.h"

#include <sstream>

using namespace Belle2;

size_t FEEModule::count() const throw()
{
  size_t c = 0;
  for (size_t i = 0; i < _reg_v.size(); i++) {
    c += _reg_v[i].count();
  }
  return c;
}

void FEEModule::addRegister(const Register& reg) throw()
{
  _reg_v.push_back(reg);
}

FEEModule::Register* FEEModule::getRegister(const std::string& name)
throw()
{
  for (size_t i = 0; i < _reg_v.size(); i++) {
    if (_reg_v[i].getName() == name) return &_reg_v[i];
  }
  return NULL;
}

const std::string FEEModule::getSQLFields() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLFields();
  for (size_t i = 0; i < _reg_v.size(); i++) {
    const std::vector<int> value_v(_reg_v[i].getValues());
    for (size_t j = 0; j < value_v.size(); j++) {
      ss << Belle2::form(", %s_%d int", _reg_v[i].getName().c_str(), j);
    }
  }
  return ss.str();
}

const std::string FEEModule::getSQLLabels() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLLabels();
  for (size_t i = 0; i < _reg_v.size(); i++) {
    const std::vector<int> value_v(_reg_v[i].getValues());
    for (size_t j = 0; j < value_v.size(); j++) {
      ss << Belle2::form(", %s_%d ", _reg_v[i].getName().c_str(), j);
    }
  }
  return ss.str();
}

const std::string FEEModule::getSQLValues() const throw()
{
  std::stringstream ss;
  ss << Module::getSQLValues();
  for (size_t i = 0; i < _reg_v.size(); i++) {
    const std::vector<int> value_v(_reg_v[i].getValues());
    for (size_t j = 0; j < value_v.size(); j++) {
      ss << ", " << value_v[j];
    }
  }
  return ss.str();
}
