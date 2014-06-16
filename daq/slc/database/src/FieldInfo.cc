#include "daq/slc/database/FieldInfo.h"

#include "daq/slc/database/ConfigInfoTable.h"
#include "daq/slc/database/LoggerInfoTable.h"
#include "daq/slc/database/DBObject.h"
#include "daq/slc/database/DBRecord.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <cstdlib>
#include <cmath>

using namespace Belle2;

template <typename T>
const std::string putNumber(T num)
{
  if (isnan(num)) return "'nan'";
  if (isinf(num)) return "'infinity'";
  std::stringstream ss;
  ss << num;
  return ss.str();
}

int FieldInfo::Property::getTypeSize() const throw()
{
  switch (m_type) {
    case BOOL:   return sizeof(bool);
    case CHAR:   return sizeof(char);
    case SHORT:  return sizeof(short);
    case INT:    return sizeof(int);
    case LONG:   return sizeof(long long);
    case FLOAT:  return sizeof(float);
    case DOUBLE: return sizeof(double);
    case NSM_CHAR:   return sizeof(char);
    case NSM_INT16:  return sizeof(short);
    case NSM_INT32:  return sizeof(int);
    case NSM_INT64:  return sizeof(long long);
    case NSM_BYTE8:  return sizeof(unsigned char);
    case NSM_UINT16: return sizeof(unsigned short);
    case NSM_UINT32: return sizeof(unsigned int);
    case NSM_UINT64: return sizeof(unsigned long long);
    case NSM_FLOAT:  return sizeof(float);
    case NSM_DOUBLE: return sizeof(double);
    default: break;
  }
  return 0;

}

const std::string FieldInfo::getTypeAlias() const throw()
{
  switch (m_type) {
    case BOOL:   return "boolean";
    case CHAR:   return "char";
    case SHORT:  return "smallint";
    case INT:    return "int";
    case LONG:   return "bigint";
    case FLOAT:  return "float";
    case DOUBLE: return "double precision";
    case TEXT:   return "text";
    case ENUM:   return "int";
    case OBJECT: return "int";
    case NSM_CHAR:   return "char";
    case NSM_INT16:  return "smallint";
    case NSM_INT32:  return "int";
    case NSM_INT64:  return "bigint";
    case NSM_BYTE8:  return "smallint";
    case NSM_UINT16: return "int";
    case NSM_UINT32: return "int";
    case NSM_UINT64: return "bigint";
    case NSM_FLOAT:  return "float";
    case NSM_DOUBLE: return "double precision";
    case NSM_OBJECT: return "int";
  }
  return "text";
}

const std::string FieldInfo::getTypeName() const throw()
{
  switch (m_type) {
    case BOOL:   return "bool";
    case CHAR:   return "char";
    case SHORT:  return "short";
    case INT:    return "int";
    case LONG:   return "long long";
    case FLOAT:  return "float";
    case DOUBLE: return "double";
    case TEXT:   return "text";
    case ENUM:   return "enum";
    case OBJECT: return "object";
    case NSM_CHAR:   return "nsm::char";
    case NSM_INT16:  return "nsm::int16";
    case NSM_INT32:  return "nsm::int32";
    case NSM_INT64:  return "nsm::int64";
    case NSM_BYTE8:  return "nsm::byte8";
    case NSM_UINT16: return "nsm::uint16";
    case NSM_UINT32: return "nsm::uint32";
    case NSM_UINT64: return "nsm::uint64";
    case NSM_FLOAT:  return "nsm::float";
    case NSM_DOUBLE: return "nsm::double";
    case NSM_OBJECT: return "nsm::object";
  }
  return "text";
}

const std::string FieldInfo::getSQL(const DBObject& obj,
                                    const std::string& name) throw()
{
  std::stringstream ss;
  const FieldInfo::Property& pro(obj.getProperty(name));
  if (pro.getLength() > 0 && (pro.getType() != OBJECT &&
                              pro.getType() != NSM_OBJECT)) {
    ConfigInfo info(obj.getName(), obj.getNode(),
                    obj.getTable() + "." + name, obj.getRevision());
    ss << "(" << info.getSQL() << ")";
  } else {
    switch (pro.getType()) {
      case BOOL:   ss << (obj.getBool(name) ? "true" : "false"); break;
      case CHAR:   ss << putNumber(obj.getChar(name)); break;
      case SHORT:  ss << putNumber(obj.getShort(name)); break;
      case INT:    ss << putNumber(obj.getInt(name)); break;
      case LONG:   ss << putNumber(obj.getLong(name)); break;
      case FLOAT:  ss << putNumber(obj.getFloat(name)); break;
      case DOUBLE: ss << putNumber(obj.getDouble(name)); break;
      case TEXT:   ss << "'" << obj.getText(name) << "'"; break;
      case ENUM:
        ss << StringUtil::form("(select fieldid from fieldid('%s', '%s', '%s'))",
                               obj.getTable().c_str(), name.c_str(),
                               obj.getEnum(name).c_str());
        break;
      case OBJECT:
        ss << "(" << getSQL(obj.getObject(name)) << ")";
        break;
      case NSM_CHAR:   ss << putNumber(obj.getChar(name)); break;
      case NSM_INT16:  ss << putNumber(obj.getShort(name)); break;
      case NSM_INT32:  ss << putNumber(obj.getInt(name)); break;
      case NSM_INT64:  ss << putNumber(obj.getLong(name)); break;
      case NSM_BYTE8:  ss << putNumber(obj.getUChar(name)); break;
      case NSM_UINT16: ss << putNumber(obj.getUShort(name)); break;
      case NSM_UINT32: ss << putNumber(obj.getUInt(name)); break;
      case NSM_UINT64: ss << putNumber(obj.getULong(name)); break;
      case NSM_FLOAT:  ss << putNumber(obj.getFloat(name)); break;
      case NSM_DOUBLE: ss << putNumber(obj.getDouble(name)); break;
      case NSM_OBJECT:
        ss << "(" << getSQL(obj.getObject(name)) << ")";
        break;
    }
  }
  return ss.str();
}

const std::string FieldInfo::getSQL(const DBObject& obj) throw()
{
  if (obj.isConfig()) {
    return ConfigInfo(obj.getName(), obj.getNode(),
                      obj.getTable(), obj.getRevision()).getSQL();
  } else {
    return LoggerInfo(obj.getNode(), obj.getTable(),
                      obj.getRevision()).getSQL();
  }
  LogFile::debug("%s:%d", __FILE__, __LINE__);
  return "";
}

const std::string FieldInfo::setSQL() const throw()
{
  std::stringstream ss;
  const std::string name = getName();
  switch (getType()) {
    case OBJECT: {
      ss << StringUtil::form("tablename_by_configinfo(%s) "
                             "as \"%s\" ", name.c_str(), name.c_str());
      /*
      ss << StringUtil::form("(select configinfo.name||','||nodeinfo.name"
           "||','||tableinfo.name||','||tableinfo.revision "
           "as \"%s\" from configinfo, nodeinfo, tableinfo "
           "where configinfo.nodeid = nodeinfo.id and "
           "nodeinfo.name = '%s' and configinfo.tableid = tableinfo.id "
           "and configinfo.name = '%s') ",
           name.c_str(), name.c_str(), name.c_str());
      */
    }; break;
    case ENUM:  {
      ss << "(select name from \"fieldinfo.type.enum\" where index = \""
         << name << "\" and fieldid = " << getId() << ") as \"" << name << "\"";
    }; break;
    default: ss << "\"" << name << "\" "; break;
  }
  return ss.str();
}

void FieldInfo::setSQL(const DBRecord& record,
                       DBObject& obj) throw()
{
  std::string name = getName();
  std::string value = record.get(name);
  switch (getType()) {
    case BOOL:   obj.addBool(name, record.getBool(name)); break;
    case CHAR:   obj.addChar(name, atoi(value.c_str())); break;
    case SHORT:  obj.addShort(name, atoi(value.c_str())); break;
    case INT:    obj.addInt(name, atoi(value.c_str())); break;
    case LONG:   obj.addLong(name, atoll(value.c_str())); break;
    case FLOAT:  obj.addFloat(name, atof(value.c_str())); break;
    case DOUBLE: obj.addDouble(name, atof(value.c_str())); break;
    case TEXT:   obj.addText(name, value); break;
    case OBJECT: break;
    case ENUM:   break;
    case NSM_CHAR:   obj.addChar(name, atoi(value.c_str())); break;
    case NSM_INT16:  obj.addShort(name, atoi(value.c_str())); break;
    case NSM_INT32:  obj.addInt(name, atoi(value.c_str())); break;
    case NSM_INT64:  obj.addLong(name, atoll(value.c_str())); break;
    case NSM_BYTE8:  obj.addUChar(name, atoi(value.c_str())); break;
    case NSM_UINT16: obj.addUShort(name, atoi(value.c_str())); break;
    case NSM_UINT32: obj.addUInt(name, atoi(value.c_str())); break;
    case NSM_UINT64: obj.addULong(name, atoll(value.c_str())); break;
    case NSM_FLOAT:  obj.addFloat(name, atof(value.c_str())); break;
    case NSM_DOUBLE: obj.addDouble(name, atof(value.c_str())); break;
    case NSM_OBJECT: break;
  }
}
