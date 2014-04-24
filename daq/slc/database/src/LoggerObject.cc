#include "daq/slc/database/LoggerObject.h"

using namespace Belle2;

LoggerObject::LoggerObject()
{
  setConfig(false);
}

LoggerObject::LoggerObject(const LoggerObject& obj)
  : DBObject(obj)
{
  setConfig(false);
  /*
  for (FieldNameList::const_iterator it = getFieldNames().begin();
       it != obj.getFieldNames().end(); it++) {
    const std::string&name(*it);
    FieldInfo::Type type = obj.getProperty(name).getType();
    switch (type) {
    case FieldInfo::BOOL:   addBool(name, obj.getBool(name)); break;
    case FieldInfo::CHAR:   addChar(name, obj.getChar(name)); break;
    case FieldInfo::SHORT:  addShort(name, obj.getShort(name)); break;
    case FieldInfo::INT:    addInt(name, obj.getInt(name)); break;
    case FieldInfo::LONG:   addLong(name, obj.getLong(name)); break;
    case FieldInfo::FLOAT:  addFloat(name, obj.getFloat(name)); break;
    case FieldInfo::DOUBLE: addDouble(name, obj.getDouble(name)); break;
    case FieldInfo::TEXT:   addText(name, obj.getText(name)); break;
    case FieldInfo::ENUM:   {
      addEnum(name, obj.getEnum(name), obj.getEnumList(name));
    } break;
    case FieldInfo::NSM_CHAR:   addChar(name, obj.getChar(name)); break;
    case FieldInfo::NSM_INT16:  addShort(name, obj.getShort(name)); break;
    case FieldInfo::NSM_INT32:  addInt(name, obj.getInt(name)); break;
    case FieldInfo::NSM_INT64:  addLong(name, obj.getLong(name)); break;
    case FieldInfo::NSM_BYTE8:  addUChar(name, obj.getChar(name)); break;
    case FieldInfo::NSM_INT16:  addUShort(name, obj.getShort(name)); break;
    case FieldInfo::NSM_INT32:  addUInt(name, obj.getInt(name)); break;
    case FieldInfo::NSM_INT64:  addULong(name, obj.getLong(name)); break;
    case FieldInfo::NSM_FLOAT:  addFloat(name, obj.getFloat(name)); break;
    case FieldInfo::NSM_DOUBLE: addDouble(name, obj.getDouble(name)); break;
    default: break;
    }
  }
  */
}

LoggerObject::~LoggerObject() throw()
{
  reset();
}


