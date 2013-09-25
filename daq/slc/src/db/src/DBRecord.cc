#include "DBRecord.hh"

#include <util/StringUtil.hh>

#include <cstdlib>

using namespace B2DAQ;

const std::string DBRecord::getFieldValue(std::string name) const throw()
{
  for (size_t i = 0; i < _name_v.size(); i++) {
    if (name == _name_v[i]) return _value_v[i];
  }
  return "";
}

const std::string DBRecord::getFieldValue(int i) const throw()
{
  if (i >= 0 && i < (int) _value_v.size()) return _value_v[i];
  return "";
}

int DBRecord::getFieldValueInt(std::string name) const throw()
{
  return atoi(getFieldValue(name).c_str());
}

int DBRecord::getFieldValueInt(int i) const throw()
{
  return atoi(getFieldValue(i).c_str());
}

std::vector<std::string> DBRecord::getFieldValueArray(std::string name) const throw()
{
  std::string value = B2DAQ::replace(B2DAQ::replace(getFieldValue(name), "}", ""), "{", "");
  return B2DAQ::split(value, ',');
}

std::vector<std::string> DBRecord::getFieldValueArray(int i) const throw()
{
  std::string value = B2DAQ::replace(B2DAQ::replace(getFieldValue(i), "}", ""), "{", "");
  return B2DAQ::split(value, ',');
}

std::vector<int> DBRecord::getFieldValueIntArray(std::string name) const throw()
{
  std::vector<int> value_i_v;
  std::vector<std::string> value_v = getFieldValueArray(name);
  for (size_t i = 0; i < value_v.size(); i++) {
    value_i_v.push_back(atoi(value_v[i].c_str()));
  }
  return value_i_v;
}

std::vector<int> DBRecord::getFieldValueIntArray(int i) const throw()
{
  std::vector<int> value_i_v;
  std::vector<std::string> value_v = getFieldValueArray(i);
  for (size_t i = 0; i < value_v.size(); i++) {
    value_i_v.push_back(atoi(value_v[i].c_str()));
  }
  return value_i_v;
}

void DBRecord::addField(std::string name, std::string value) throw()
{
  _name_v.push_back(name);
  _value_v.push_back(value);
}

const std::string DBRecord::getFieldName(int i) const throw()
{
  if (i >= 0 && i < (int) _name_v.size()) return _name_v[i];
  return "";
}

