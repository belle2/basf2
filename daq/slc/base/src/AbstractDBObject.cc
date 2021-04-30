#include "daq/slc/base/AbstractDBObject.h"

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <limits>

using namespace Belle2;

AbstractDBObject::AbstractDBObject() : m_index(0), m_path(), m_id(0)
{

}

AbstractDBObject::AbstractDBObject(const AbstractDBObject& obj)
  : m_index(obj.m_index), m_path(obj.m_path),
    m_id(obj.m_id),
    m_name(obj.m_name) {}

AbstractDBObject::~AbstractDBObject()
{
  reset();
}

void AbstractDBObject::reset()
{
  m_path = "";
  m_id = 0;
  m_index = 0;
  m_name_v = DBField::NameList();
  m_pro_m = DBField::PropertyList();
}

DBField::Property AbstractDBObject::getProperty(const std::string& name) const
{
  DBField::PropertyList::const_iterator it = m_pro_m.find(name);
  if (it != m_pro_m.end()) return it->second;
  return DBField::Property();
}

bool AbstractDBObject::hasField(const std::string& name) const
{
  DBField::PropertyList::const_iterator it = m_pro_m.find(name);
  return (it != m_pro_m.end());
}

bool AbstractDBObject::hasValue(const std::string& name) const
{
  DBField::PropertyList::const_iterator it = m_pro_m.find(name);
  return hasField(name) &&
         it->second.getType() != DBField::TEXT &&
         it->second.getType() != DBField::OBJECT;
}

bool AbstractDBObject::hasText(const std::string& name) const
{
  DBField::PropertyList::const_iterator it = m_pro_m.find(name);
  return hasField(name) && it->second.getType() == DBField::TEXT;
}

bool AbstractDBObject::hasObject(const std::string& name) const
{
  DBField::PropertyList::const_iterator it = m_pro_m.find(name);
  return hasField(name) &&
         it->second.getType() == DBField::OBJECT;
}

void AbstractDBObject::add(const std::string& name, DBField::Property pro)
{
  if (!hasField(name)) {
    m_name_v.push_back(name);
    m_pro_m.insert(DBField::PropertyList::value_type(name, pro));
  }
}

const std::string AbstractDBObject::getValueText(const std::string& name)
const
{
  const static int double_precision = std::numeric_limits<double>::digits10 + 2;

  if (hasField(name)) {
    switch (getProperty(name).getType()) {
      case DBField::BOOL:   return getBool(name) ? "true" : "false";
      case DBField::CHAR:   return StringUtil::form("%d", (int)getChar(name));
      case DBField::SHORT:  return StringUtil::form("%d", (int)getShort(name));
      case DBField::INT:    return StringUtil::form("%d", (int)getInt(name));
      case DBField::LONG:   return StringUtil::form("%ld", getLong(name));
      case DBField::FLOAT:  return StringUtil::form("%f", getFloat(name));
      case DBField::DOUBLE: return StringUtil::form("%.*e", double_precision, getDouble(name));
      case DBField::TEXT:   return getText(name);
      default: break;
    }
  }
  throw (std::out_of_range(name + " not found"));
}

void AbstractDBObject::setValueText(const std::string& name,
                                    const std::string& value)
{
  if (hasField(name)) {
    switch (getProperty(name).getType()) {
      case DBField::BOOL:       setBool(name, value == "true" || value == "t"); break;
      case DBField::CHAR: {
        if (StringUtil::find(value, "0x"))
          setChar(name, (char)strtol(value.c_str(), NULL, 0));
        else
          setChar(name, (char)atoi(value.c_str()));
      } break;
      case DBField::SHORT: {
        if (StringUtil::find(value, "0x"))
          setShort(name, (short)strtol(value.c_str(), NULL, 0));
        else
          setShort(name, (short)atoi(value.c_str()));
      } break;
      case DBField::INT: {
        if (StringUtil::find(value, "0x"))
          setInt(name, (int)strtol(value.c_str(), NULL, 0));
        else
          setInt(name, (int)atoi(value.c_str()));
      } break;
      case DBField::LONG:       setLong(name, (long long)atoi(value.c_str())); break;
      case DBField::FLOAT:      setFloat(name, atof(value.c_str())); break;
      case DBField::DOUBLE:     setDouble(name, atof(value.c_str())); break;
      case DBField::TEXT:       setText(name, value); break;
      default: break;
    }
    return;
  }
  throw (std::out_of_range(name + " not found"));
}

