#include "XMLClass.hh"

#include <cstdlib>

using namespace B2DAQ;

void XMLClass::add(const std::string& type,
                   const std::string& label,
                   const std::string& value)
{
  ValueType vtype;
  if (type.find("text") != std::string::npos) {
    if (type == "text" || type == "text32") {
      vtype = TEXT32;
    } else if (type == "text8") {
      vtype = TEXT8;
    } else if (type == "text16") {
      vtype = TEXT16;
    } else if (type == "text64") {
      vtype = TEXT64;
    } else if (type == "text256") {
      vtype = TEXT256;
    }
    _text_v.insert(std::map<std::string, std::string>::value_type(label, value));
  } else if (type.find("int") != std::string::npos) {
    if (type == "int" || type == "int32") {
      vtype = INT32;
    } else if (type == "int8") {
      vtype = INT8;
    } else if (type == "int16") {
      vtype = INT16;
    } else if (type == "int64") {
      vtype = INT64;
    }
    _int_v.insert(std::map<std::string, int>::value_type(label, atoi(value.c_str())));
  } else if (type.find("float") != std::string::npos) {
    if (type == "float" || type == "float32") {
      vtype = FLOAT32;
    } else if (type == "float64") {
      vtype = FLOAT64;
    }
    _float_v.insert(std::map<std::string, double>::value_type(label, atof(value.c_str())));
  } else if (type == "boolean") {
    vtype = BOOLEAN;
    _boolean_v.insert(std::map<std::string, bool>::value_type(label, (bool)atoi(value.c_str())));
  }
  _type_v.insert(std::map<std::string, ValueType>::value_type(label, vtype));
}
