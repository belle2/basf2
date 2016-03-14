#include "daq/slc/database/DBObject.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace Belle2;

DBObject::DBObject()
{
}

DBObject::DBObject(const std::string& path)
{
  setPath(path);
}

DBObject::DBObject(const DBObject& obj)
  : AbstractDBObject(obj)
{
  copy(obj);
}

const DBObject& DBObject::operator=(const DBObject& obj) throw()
{
  copy(obj);
  return *this;
}

void DBObject::copy(const DBObject& obj)
{
  reset();
  setIndex(obj.getIndex());
  setId(obj.getId());
  setPath(obj.getPath());
  setName(obj.getName());
  for (FieldNameList::const_iterator it = obj.getFieldNames().begin();
       it != obj.getFieldNames().end(); it++) {
    const std::string& name(*it);
    DBField::Type type = obj.getProperty(name).getType();
    switch (type) {
      case DBField::BOOL:   addBool(name, obj.getBool(name)); break;
      case DBField::CHAR:   addChar(name, obj.getChar(name)); break;
      case DBField::SHORT:  addShort(name, obj.getShort(name)); break;
      case DBField::INT:    addInt(name, obj.getInt(name)); break;
      case DBField::LONG:   addLong(name, obj.getLong(name)); break;
      case DBField::FLOAT:  addFloat(name, obj.getFloat(name)); break;
      case DBField::DOUBLE: addDouble(name, obj.getDouble(name)); break;
      case DBField::TEXT:   addText(name, obj.getText(name)); break;
      case DBField::OBJECT: addObjects(name, obj.getObjects(name)); break;
      default: break;
    }
  }
}

DBObject::~DBObject() throw()
{
  reset();
}

int DBObject::getNObjects(const std::string& name) const throw()
{
  FieldObjectList::const_iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second.size();
  return 0;
}

DBObjectList& DBObject::getObjects(const std::string& name) throw(std::out_of_range)
{
  FieldObjectList::iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

const DBObjectList& DBObject::getObjects(const std::string& name) const throw(std::out_of_range)
{
  FieldObjectList::const_iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

DBObject& DBObject::getObject(const std::string& name, int i) throw(std::out_of_range)
{
  FieldObjectList::iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second[i];
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

const DBObject& DBObject::getObject(const std::string& name, int i) const throw(std::out_of_range)
{
  FieldObjectList::const_iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second[i];
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

void DBObject::reset() throw()
{
  const FieldNameList& name_v(getFieldNames());
  for (size_t ii = 0; ii < name_v.size(); ii++) {
    const std::string& name(name_v[ii]);
    if (hasObject(name)) {
      size_t nobj = getNObjects(name);
      for (size_t i = 0; i < nobj; i++) getObject(name, i).reset();
    }
  }
  for (FieldValueList::iterator it = m_value_m.begin();
       it != m_value_m.end(); it++) {
    free(it->second);
  }
  m_value_m = FieldValueList();
  m_text_m = FieldTextList();
  m_obj_v_m = FieldObjectList();
  AbstractDBObject::reset();
}

void DBObject::readObject(Reader& reader) throw(IOException)
{
  reset();
  setPath(reader.readString());
  setName(reader.readString());
  int npar = reader.readInt();
  for (int i = 0; i < npar; i++) {
    std::string name = reader.readString();
    DBField::Type type = (DBField::Type)reader.readInt();
    switch (type) {
      case DBField::BOOL:   addBool(name, reader.readBool()); break;
      case DBField::CHAR:   addChar(name, reader.readChar()); break;
      case DBField::SHORT:  addShort(name, reader.readShort()); break;
      case DBField::INT:    addInt(name, reader.readInt()); break;
      case DBField::LONG:   addLong(name, reader.readLong()); break;
      case DBField::FLOAT:  addFloat(name, reader.readFloat()); break;
      case DBField::DOUBLE: addDouble(name, reader.readDouble()); break;
      case DBField::TEXT:   addText(name, reader.readString()); break;
      case DBField::OBJECT: {
        DBObjectList obj_v;
        int nobj = reader.readInt();
        for (int n = 0; n < nobj; n++) {
          DBObject obj;
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

void DBObject::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(getPath());
  writer.writeString(getName());
  const FieldNameList& name_v(getFieldNames());
  writer.writeInt(name_v.size());
  for (FieldNameList::const_iterator iname = name_v.begin();
       iname != name_v.end(); iname++) {
    const std::string name = *iname;
    DBField::Type type = getProperty(name).getType();
    writer.writeString(name);
    writer.writeInt(type);
    switch (type) {
      case DBField::BOOL:   writer.writeBool(getBool(name)); break;
      case DBField::CHAR:   writer.writeChar(getChar(name)); break;
      case DBField::SHORT:  writer.writeShort(getShort(name)); break;
      case DBField::INT:    writer.writeInt(getInt(name)); break;
      case DBField::LONG:   writer.writeLong(getLong(name)); break;
      case DBField::FLOAT:  writer.writeFloat(getFloat(name)); break;
      case DBField::DOUBLE:  writer.writeDouble(getDouble(name)); break;
      case DBField::TEXT:   writer.writeString(getText(name)); break;
      case DBField::OBJECT:  {
        const DBObjectList& obj_v(getObjects(name));
        writer.writeInt(obj_v.size());
        for (DBObjectList::const_iterator iobj = obj_v.begin();
             iobj != obj_v.end(); iobj++) {
          iobj->writeObject(writer);
        }
      }; break;
      default: break;
    }
  }
}

const void* DBObject::getValue(const std::string& name) const throw(std::out_of_range)
{
  if (!hasValue(name)) return NULL;
  FieldValueList::const_iterator it = m_value_m.find(name);
  if (it != m_value_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s:%d", __FILE__, __LINE__)));
}

const std::string& DBObject::getText(const std::string& name) const throw(std::out_of_range)
{
  if (!hasText(name)) return m_empty;
  FieldTextList::const_iterator it = m_text_m.find(name);
  if (it != m_text_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s:%d", __FILE__, __LINE__)));
}

void DBObject::addValue(const std::string& name, const void* value,
                        DBField::Type type, int) throw()
{
  DBField::Property pro(type, 0, 0);
  int size = pro.getTypeSize();
  if (size <= 0) return;
  if (!hasValue(name)) {
    add(name, pro);
    void* v = malloc(size);
    memcpy(v, value, size);
    m_value_m.insert(FieldValueList::value_type(name, v));
  } else {
    memcpy(m_value_m[name], value, size);
  }
}

void DBObject::setValue(const std::string& name,
                        const void* value, int) throw()
{
  const DBField::Property& pro(getProperty(name));
  int size = pro.getTypeSize();
  if (hasField(name) && size > 0) {
    memcpy(m_value_m[name], value, size);
  }
}

void DBObject::addText(const std::string& name,
                       const std::string& value) throw()
{
  if (!hasField(name)) {
    add(name, DBField::Property(DBField::TEXT, 0));
    m_text_m.insert(FieldTextList::value_type(name, value));
  } else {
    m_text_m[name] = value;
  }
}

void DBObject::addObject(const std::string& name,
                         const DBObject& obj) throw()
{
  if (!hasField(name)) {
    add(name, DBField::Property(DBField::OBJECT, 0));
    m_obj_v_m.insert(FieldObjectList::value_type(name, DBObjectList()));
  }
  m_obj_v_m[name].push_back(obj);
  getProperty(name).setLength(m_obj_v_m[name].size());
}

void DBObject::addObjects(const std::string& name,
                          const DBObjectList& obj_v) throw()
{
  if (!hasField(name)) {
    add(name, DBField::Property(DBField::OBJECT, obj_v.size()));
    m_obj_v_m.insert(FieldObjectList::value_type(name, obj_v));
  } else {
    m_obj_v_m[name] = obj_v;
  }
}

void DBObject::print(bool isfull) const throw()
{
  const std::string& name_in = "";
  NameValueList map;
  search(map, name_in, isfull);
  size_t length = 0;
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    if (it->name.size() > length) length = it->name.size();
  }
  printf("#\n");
  printf("# Config object (confname = %s)\n", getName().c_str());
  printf("#\n");
  printf("\n");
  StringList s = StringUtil::split(getName(), '@');
  if (s.size() > 1) {
    printf(StringUtil::form("%%-%ds : %%s\n", length).c_str(),
           "nodename", s[0].c_str());
    printf(StringUtil::form("%%-%ds : %%s\n", length).c_str(),
           "config", s[1].c_str());
  } else {
    printf(StringUtil::form("%%-%ds : %%s\n", length).c_str(),
           "config", getName().c_str());
  }
  printf("\n");
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    printf(StringUtil::form("%%-%ds : %%s\n", length).c_str(),
           it->name.c_str(), it->value.c_str());
  }
  printf("\n");
  printf("#\n");
  printf("#\n");
  printf("#\n");
}

StringList DBObject::getNameList(bool isfull) const throw()
{
  const std::string& name_in = "";
  NameValueList map;
  search(map, name_in, isfull);
  StringList str;
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    str.push_back(it->name);
  }
  return str;
}

void DBObject::search(NameValueList& map, const std::string& name_in, bool isfull)
const throw()
{
  const FieldNameList& name_v(getFieldNames());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const DBField::Property& pro(getProperty(name));
    std::string name_out = name_in;
    if (name_in.size() > 0) name_out += ".";
    name_out += name;
    std::string ptype;
    switch (pro.getType()) {
      case DBField::BOOL: ptype = "bool"; break;
      case DBField::CHAR:  ptype = "char"; break;
      case DBField::SHORT: ptype = "short"; break;
      case DBField::INT:  ptype = "int"; break;
      case DBField::LONG:  ptype = "long"; break;
      case DBField::FLOAT:  ptype = "float"; break;
      case DBField::DOUBLE: ptype = "double"; break;
      default : break;
    }
    if (pro.getType() == DBField::OBJECT) {
      int length = getNObjects(name);
      if (!isfull && getObject(name).getPath().size() > 0) {
        const DBObjectList& objs(getObjects(name));
        if (length == 1 || objs[1].getPath().size() == 0 || objs[0].getPath() == objs[1].getPath()) {
          std::string value = objs[0].getPath();
          value = "object(" + value + ")";
          NameValue nv;
          nv.name = name_out.c_str();
          nv.value = value;
          nv.type = pro.getType();
          map.push_back(nv);
        } else {
          for (int i = 0; i < length; i++) {
            std::string value = objs[i].getPath();
            value = "object(" + value + ")";
            NameValue nv;
            nv.name = StringUtil::form("%s[%d].%s", name.c_str(), i, name_out.c_str());
            nv.value = value;
            nv.type = pro.getType();
            map.push_back(nv);
          }
        }
      } else {
        size_t length = getNObjects(name);
        if (length > 1) {
          const DBObjectList& objs(getObjects(name));
          for (size_t i = 0; i < length; i++) {
            objs[i].search(map, StringUtil::form("%s[%d]", name_out.c_str(), i), isfull);
          }
        } else {
          const DBObject& obj(getObject(name));
          obj.search(map, name_out, isfull);
        }
      }
    } else {
      std::string value = getValueText(name);
      if (ptype.size() > 0) value = ptype + "(" + value + ")";
      NameValue nv;
      nv.name = name_out;
      nv.value = value;
      nv.type = pro.getType();
      if (pro.getType() != DBField::TEXT) {
        nv.buf = (void*)getValue(name);
      } else {
        nv.buf = (void*)&getText(name);
      }
      map.push_back(nv);
    }
  }
}

void DBObject::printSQL(const std::string& table, std::ostream& out,
                        const std::string& name_inc, int index)  const throw()
{
  const std::string& name_in = (name_inc.size() == 0) ? getName() : name_inc;
  if (index >= 0) {
    out << StringUtil::form("insert into %s (name, path) values ('%s[%d]', '.%s.');",
                            table.c_str(), getName().c_str(), index, name_in.c_str()) << std::endl;
  } else {
    out << StringUtil::form("insert into %s (name, path) values ('%s', '.%s.');",
                            table.c_str(), getName().c_str(), name_in.c_str()) << std::endl;
  }
  const FieldNameList& name_v(getFieldNames());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const DBField::Property& pro(getProperty(name));
    std::string name_out = name_in;
    if (name_in.size() > 0) name_out += ".";
    name_out += name;
    std::string ptype;
    switch (pro.getType()) {
      case DBField::BOOL: ptype = "value_b"; break;
      case DBField::CHAR:  ptype = "value_c"; break;
      case DBField::SHORT: ptype = "value_s"; break;
      case DBField::INT:  ptype = "value_i"; break;
      case DBField::LONG:  ptype = "value_l"; break;
      case DBField::FLOAT:  ptype = "value_f"; break;
      case DBField::DOUBLE: ptype = "value_d"; break;
      case DBField::TEXT: ptype = "value_t"; break;
      default : break;
    }
    if (pro.getType() == DBField::OBJECT) {
      int length = getNObjects(name);
      if (length > 1) {
        const DBObjectList& objs(getObjects(name));
        if (objs[0].getPath().size() > 0) {
          if (objs[1].getPath().size() == 0 || objs[0].getPath() == objs[1].getPath()) {
            out << StringUtil::form("insert into %s (name, path, value_o) values ('%s', '.%s.', '%s');",
                                    table.c_str(), objs[0].getName().c_str(),
                                    name_out.c_str(), objs[0].getPath().c_str()) << std::endl;
          } else {
            for (int i = 0; i < length; i++) {
              out << StringUtil::form("insert into %s (name, path, value_o) values ('%s[%d]', '.%s[%d].', '%s');",
                                      table.c_str(), objs[0].getName().c_str(),
                                      i, name_out.c_str(), i, objs[0].getPath().c_str()) << std::endl;
            }
          }
        } else {
          for (int i = 0; i < length; i++) {
            objs[i].printSQL(table, out, StringUtil::form("%s[%d]", name_out.c_str(), i), i);
          }
        }
      } else {
        const DBObject& obj(getObject(name));
        if (obj.getPath().size() > 0) {
          out << StringUtil::form("insert into %s (name, path, value_o) values ('%s', '.%s.', '%s');",
                                  table.c_str(), obj.getName().c_str(),
                                  name_out.c_str(), obj.getPath().c_str()) << std::endl;
        } else {
          obj.printSQL(table, out, name_out);
        }
      }
    } else {
      std::string value = getValueText(name);
      if (pro.getType() == DBField::TEXT) value = "'" + value + "'";
      out << StringUtil::form("insert into %s (name, path, %s) values ('%s', '.%s.', %s);",
                              table.c_str(), ptype.c_str(), name.c_str(),
                              name_out.c_str(), value.c_str()) << std::endl;
    }
  }
}

