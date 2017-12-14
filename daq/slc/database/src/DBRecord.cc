#include "daq/slc/database/DBRecord.h"

#include "daq/slc/base/StringUtil.h"

#include <cstdlib>

using namespace Belle2;

const std::string DBRecord::get(const std::string& name) const throw()
{
  if (m_value_m.find(name) != m_value_m.end()) return m_value_m[name];
  else return "";
}

const std::string DBRecord::get(int i) const throw()
{
  if (i >= 0 && i < (int)m_name_v.size())
    return m_value_m[m_name_v[i]];
  else return "";
}

int DBRecord::getInt(const std::string& name) const throw()
{
  return atoi(get(name).c_str());
}

int DBRecord::getInt(int i) const throw()
{
  return atoi(get(i).c_str());
}

bool DBRecord::getBool(const std::string& name) const throw()
{
  return get(name) == "t";
}

bool DBRecord::getBool(int i) const throw()
{
  return get(i) == "t";
}

float DBRecord::getFloat(const std::string& name) const throw()
{
  return atof(get(name).c_str());
}

float DBRecord::getFloat(int i) const throw()
{
  return atof(get(i).c_str());
}

std::vector<std::string> DBRecord::getArray(const std::string& name) const throw()
{
  std::string value = StringUtil::replace(StringUtil::replace(get(name), "}", ""), "{", "");
  return StringUtil::split(value, ',');
}

std::vector<std::string> DBRecord::getArray(int i) const throw()
{
  std::string value = StringUtil::replace(StringUtil::replace(get(i), "}", ""), "{", "");
  return StringUtil::split(value, ',');
}

std::vector<int> DBRecord::getIntArray(const std::string& name) const throw()
{
  std::vector<int> value_i_v;
  std::vector<std::string> value_v = getArray(name);
  for (size_t i = 0; i < value_v.size(); i++) {
    value_i_v.push_back(atoi(value_v[i].c_str()));
  }
  return value_i_v;
}

std::vector<int> DBRecord::getIntArray(int i) const throw()
{
  std::vector<int> value_i_v;
  std::vector<std::string> value_v = getArray(i);
  for (size_t i = 0; i < value_v.size(); i++) {
    value_i_v.push_back(atoi(value_v[i].c_str()));
  }
  return value_i_v;
}

std::vector<float> DBRecord::getFloatArray(const std::string& name) const throw()
{
  std::vector<float> value_i_v;
  std::vector<std::string> value_v = getArray(name);
  for (size_t i = 0; i < value_v.size(); i++) {
    value_i_v.push_back(atof(value_v[i].c_str()));
  }
  return value_i_v;
}

std::vector<float> DBRecord::getFloatArray(int i) const throw()
{
  std::vector<float> value_i_v;
  std::vector<std::string> value_v = getArray(i);
  for (size_t i = 0; i < value_v.size(); i++) {
    value_i_v.push_back(atof(value_v[i].c_str()));
  }
  return value_i_v;
}

void DBRecord::add(std::string name, std::string value) throw()
{
  m_name_v.push_back(name);
  m_value_m.insert(DBFieldList::value_type(name, value));
}

const std::string DBRecord::getFieldName(int i) const throw()
{
  if (i >= 0 && i < (int) m_name_v.size()) return m_name_v[i];
  return "";
}

