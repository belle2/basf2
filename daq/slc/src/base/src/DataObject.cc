#include "base/DataObject.h"
#include "base/ConfigFile.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace Belle2;

DataObject::DataObject()
{
}

DataObject::DataObject(const std::string& data_class,
                       const std::string& base_class)
{
  _class = data_class;
  _base_class = base_class;
}

int DataObject::setToMessage(ParamPriority priority, unsigned int* pars, int par_i,
                             std::vector<std::string>& datap)
{
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    if (priority != _param_priority_v[i]) continue;
    std::string& name(_param_name_v[i]);
    switch (_param_type_v[i]) {
      case BOOLEAN:
        pars[par_i++] = (int)_bool_value_m[name];
        break;
      case ENUM:
      case INT:
        pars[par_i++] = _int_value_m[name];
        break;
      case TEXT:
        datap.push_back(_text_value_m[name]);
        break;
      case OBJECT:
        par_i = _object_m[name]->setToMessage(priority, pars, par_i, datap);
        break;
    }
  }
  return par_i;
}

int DataObject::setToMessage(const Command& command, unsigned int* pars,
                             int par_i, std::vector<std::string>& datap)
{
  ParamPriority priority = LOAD;
  if (command == Command::BOOT) priority = BOOT;
  else if (command == Command::TRIGFT) priority = TRIGFT;
  return setToMessage(priority, pars, par_i, datap);
}

int DataObject::getFromMessage(ParamPriority priority, const unsigned int* pars, int par_i,
                               std::vector<std::string>& datap)
{
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    if (priority != _param_priority_v[i]) continue;
    std::string& name(_param_name_v[i]);
    switch (_param_type_v[i]) {
      case BOOLEAN:
        _bool_value_m[name] = (bool)pars[par_i++];
        break;
      case ENUM:
      case INT:
        _int_value_m[name] = pars[par_i++];
        break;
      case TEXT:
        _text_value_m[name] = datap[0];
        datap.erase(datap.begin());
        break;
      case OBJECT:
        par_i = _object_m[name]->getFromMessage(priority, pars, par_i, datap);
        break;
    }
  }
  return par_i;
}

int DataObject::getFromMessage(const Command& command, const unsigned int* pars,
                               int par_i, std::vector<std::string>& datap)
{
  ParamPriority priority = LOAD;
  if (command == Command::BOOT) priority = BOOT;
  else if (command == Command::TRIGFT) priority = TRIGFT;
  return getFromMessage(priority, pars, par_i, datap);
}

void DataObject::print()
{
  std::cout << _class << " rev = " << _revision << std::endl;
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    std::string& name(_param_name_v[i]);
    std::string priority_s = "LOAD";
    switch (_param_priority_v[i]) {
      case BOOT: priority_s = "BOOT"; break;
      case LOAD: priority_s = "LOAD"; break;
      case TRIGFT: priority_s = "TRIGFT"; break;
      default: break;
    }
    std::cout << name << " " << priority_s << " ";
    switch (_param_type_v[i]) {
      case BOOLEAN:
        std::cout << _bool_value_m[name] << std::endl;
        break;
      case ENUM:
      case INT:
        std::cout << _int_value_m[name] << std::endl;
        break;
      case TEXT:
        std::cout << _text_value_m[name] << std::endl;
        break;
      case OBJECT:
        _object_m[name]->print();
        break;
    }
  }
}

const std::string DataObject::toSQLConfig()
{
  std::stringstream ss;
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    std::string name = _param_name_v[i];
    switch (_param_type_v[i]) {
      case BOOLEAN:   ss << ", " << name << " boolean"; break;
      case ENUM:
      case INT:  ss << ", " << name << " int"; break;
      case TEXT: ss << ", " << name << " text"; break;
      default : break;
    }
  }
  return ss.str();
}

const std::string DataObject::toSQLNames()
{
  std::stringstream ss;
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    std::string name = _param_name_v[i];
    switch (_param_type_v[i]) {
      case BOOLEAN:
      case ENUM:
      case INT:
      case TEXT:
        ss << ", " << name; break;
      default : break;
    }
  }
  return ss.str();
}

const std::string DataObject::toSQLValues()
{
  std::stringstream ss;
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    std::string name = _param_name_v[i];
    switch (_param_type_v[i]) {
      case BOOLEAN:   ss << ", " << _bool_value_m[name]; break;
      case ENUM:
      case INT:  ss << ", " << _int_value_m[name]; break;
      case TEXT: ss << ", '" << _text_value_m[name] << "'"; break;
      default : break;
    }
  }
  return ss.str();
}

const std::string DataObject::getValueString()
{
  std::stringstream ss;
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    std::string name = _param_name_v[i];
    switch (_param_type_v[i]) {
      case BOOLEAN:
        ss << name << " : " << _bool_value_m[name] << "" << std::endl; break;
      case ENUM:
      case INT:
        ss << name << " : " << _int_value_m[name] << "" << std::endl; break;
      case TEXT:
        ss << name << " : \"" << _text_value_m[name] << "\"" << std::endl; break;
      default : break;
    }
  }
  return ss.str();
}

void DataObject::readValueString(ConfigFile& config)
{
  for (size_t i = 0; i < _param_name_v.size(); i++) {
    std::string name = _param_name_v[i];
    switch (_param_type_v[i]) {
      case BOOLEAN:
        _bool_value_m[name] = (bool)config.getInt(name); break;
      case ENUM:
      case INT:
        _int_value_m[name] = config.getInt(name); break;
      case TEXT:
        _text_value_m[name] = config.get(name); break;
      default : break;
    }
  }
}

void DataObject::setSQLValues(std::vector<std::string>& name_v,
                              std::vector<std::string>& value_v)
{
  std::stringstream ss;
  for (size_t i = 0; i < name_v.size(); i++) {
    std::string name = name_v[i];
    if (_bool_value_m.find(name) != _bool_value_m.end()) {
      _bool_value_m[name] = (bool)atoi(value_v[i].c_str());
    } else if (_int_value_m.find(name) != _int_value_m.end()) {
      _int_value_m[name] = atoi(value_v[i].c_str());
    } else if (_text_value_m.find(name) != _text_value_m.end()) {
      _text_value_m[name] = value_v[i];
    }
  }
}
