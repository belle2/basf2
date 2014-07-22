#include "daq/slc/database/ConfigObject.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

//#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <cstring>

using namespace Belle2;

ConfigObject::ConfigObject()
{
}

ConfigObject::ConfigObject(const ConfigObject& obj)
  : DBObject(obj)
{
  copy(obj);
}

const ConfigObject& ConfigObject::operator=(const ConfigObject& obj) throw()
{
  copy(obj);
  return *this;
}

void ConfigObject::copy(const ConfigObject& obj)
{
  reset();
  setIndex(obj.getIndex());
  setId(obj.getId());
  setRevision(obj.getRevision());
  setName(obj.getName());
  setNode(obj.getNode());
  setTable(obj.getTable());
  setConfig(obj.isConfig());
  for (FieldNameList::const_iterator it = obj.getFieldNames().begin();
       it != obj.getFieldNames().end(); it++) {
    const std::string& name(*it);
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
      case FieldInfo::OBJECT: addObjects(name, obj.getObjects(name)); break;
      default: break;
    }
  }
}

ConfigObject::~ConfigObject() throw()
{
  reset();
}

void ConfigObject::reset() throw()
{
  DBObject::reset();
  for (FieldValueList::iterator it = m_value_m.begin();
       it != m_value_m.end(); it++) {
    free(it->second);
  }
  m_value_m = FieldValueList();
  m_text_m = FieldTextList();
  m_obj_v_m = FieldObjectList();
}

void ConfigObject::readObject(Reader& reader) throw(IOException)
{
  reset();
  setId(reader.readInt());
  setName(reader.readString());
  setNode(reader.readString());
  setTable(reader.readString());
  setRevision(reader.readInt());
  int npar = reader.readInt();
  for (int i = 0; i < npar; i++) {
    std::string name = reader.readString();
    FieldInfo::Type type = (FieldInfo::Type)reader.readInt();
    switch (type) {
      case FieldInfo::BOOL:   addBool(name, reader.readBool()); break;
      case FieldInfo::CHAR:   addChar(name, reader.readChar()); break;
      case FieldInfo::SHORT:  addShort(name, reader.readShort()); break;
      case FieldInfo::INT:    addInt(name, reader.readInt()); break;
      case FieldInfo::LONG:   addLong(name, reader.readLong()); break;
      case FieldInfo::FLOAT:  addFloat(name, reader.readFloat()); break;
      case FieldInfo::DOUBLE: addDouble(name, reader.readDouble()); break;
      case FieldInfo::TEXT:   addText(name, reader.readString()); break;
      case FieldInfo::OBJECT: {
        ConfigObjectList obj_v;
        int nobj = reader.readInt();
        for (int n = 0; n < nobj; n++) {
          ConfigObject obj;
          obj.readObject(reader);
          obj.setIndex(n);
          obj_v.push_back(obj);
        }
        addObjects(name, obj_v);
      }; break;
      default: break;
    }
  }
}

void ConfigObject::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt(getId());
  writer.writeString(getName());
  writer.writeString(getNode());
  writer.writeString(getTable());
  writer.writeInt(getRevision());
  const FieldNameList& name_v(getFieldNames());
  writer.writeInt(name_v.size());
  for (size_t ii = 0; ii < name_v.size(); ii++) {
    std::string name = name_v.at(ii);
    FieldInfo::Type type = getProperty(name).getType();
    writer.writeString(name);
    writer.writeInt(type);
    switch (type) {
      case FieldInfo::BOOL:   writer.writeBool(getBool(name)); break;
      case FieldInfo::CHAR:   writer.writeChar(getChar(name)); break;
      case FieldInfo::SHORT:  writer.writeShort(getShort(name)); break;
      case FieldInfo::INT:    writer.writeInt(getInt(name)); break;
      case FieldInfo::LONG:   writer.writeLong(getLong(name)); break;
      case FieldInfo::FLOAT:  writer.writeFloat(getFloat(name)); break;
      case FieldInfo::DOUBLE:  writer.writeDouble(getDouble(name)); break;
      case FieldInfo::TEXT:   writer.writeString(getText(name)); break;
      case FieldInfo::OBJECT:  {
        ConfigObjectList& obj_v(m_obj_v_m[name]);
        writer.writeInt(obj_v.size());
        for (size_t n = 0; n < obj_v.size(); n++) {
          obj_v[n].writeObject(writer);
        }
      }; break;
      default: break;
    }
  }
}

const void* ConfigObject::getValue(const std::string& name) const throw()
{
  if (!hasValue(name)) return NULL;
  return m_value_m[name];
}

const std::string ConfigObject::getText(const std::string& name) const throw()
{
  if (!hasText(name)) return "";
  return m_text_m[name];
}

void ConfigObject::addValue(const std::string& name, const void* value,
                            FieldInfo::Type type, int) throw()
{
  FieldInfo::Property pro(type, 0, 0);
  int size = pro.getTypeSize();
  if (size <= 0) return;
  if (!hasValue(name)) {
    add(name, pro);
    //LogFile::debug("malloc(%d) >> %s", size, name.c_str());
    void* v = malloc(size);
    memcpy(v, value, size);
    m_value_m.insert(FieldValueList::value_type(name, v));
  } else {
    memcpy(m_value_m[name], value, size);
  }
}

void ConfigObject::setValue(const std::string& name,
                            const void* value, int) throw()
{
  const FieldInfo::Property& pro(getProperty(name));
  int size = pro.getTypeSize();
  if (hasField(name) && size > 0) {
    memcpy(m_value_m[name], value, size);
  }
}

void ConfigObject::addText(const std::string& name,
                           const std::string& value) throw()
{
  if (!hasField(name)) {
    add(name, FieldInfo::Property(FieldInfo::TEXT, 0));
    m_text_m.insert(FieldTextList::value_type(name, value));
  } else {
    m_text_m[name] = value;
  }
}

void ConfigObject::addObject(const std::string& name,
                             const ConfigObject& obj) throw()
{
  if (!hasField(name)) {
    add(name, FieldInfo::Property(FieldInfo::OBJECT, 0));
    m_obj_v_m.insert(FieldObjectList::value_type(name, ConfigObjectList()));
  }
  m_obj_v_m[name].push_back(obj);
  getProperty(name).setLength(m_obj_v_m[name].size());
}

void ConfigObject::addObjects(const std::string& name,
                              const ConfigObjectList& obj_v) throw()
{
  if (!hasField(name)) {
    add(name, FieldInfo::Property(FieldInfo::OBJECT, obj_v.size()));
    m_obj_v_m.insert(FieldObjectList::value_type(name, obj_v));
  } else {
    m_obj_v_m[name] = obj_v;
  }
}

void ConfigObject::setObject(const std::string& name,
                             int index, const DBObject& obj) throw()
{
  if (hasObject(name, index)) {
    m_obj_v_m[name][index] = (const ConfigObject&)obj;
  }
}
