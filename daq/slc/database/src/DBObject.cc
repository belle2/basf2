/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/database/DBObject.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace Belle2;

DBObject::DBObject()
{
  setDate(0);
}

DBObject::DBObject(const std::string& path)
{
  setDate(0);
  setPath(path);
}

DBObject::DBObject(const DBObject& obj)
  : AbstractDBObject(obj)
{
  copy(obj);
}

const DBObject& DBObject::operator=(const DBObject& obj)
{
  copy(obj);
  return *this;
}

void DBObject::copy(const DBObject& obj)
{
  reset();
  setDate(obj.getDate());
  setIndex(obj.getIndex());
  setId(obj.getId());
  setPath(obj.getPath());
  setName(obj.getName());
  for (DBField::NameList::const_iterator it = obj.getFieldNames().begin();
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

DBObject::~DBObject()
{
  reset();
}

int DBObject::getNObjects(const std::string& name) const
{
  FieldObjectList::const_iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second.size();
  return 0;
}

DBObjectList& DBObject::getObjects(const std::string& name)
{
  FieldObjectList::iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

const DBObjectList& DBObject::getObjects(const std::string& name) const
{
  FieldObjectList::const_iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

DBObject& DBObject::getObject(const std::string& name, int i)
{
  FieldObjectList::iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second[i];
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

const DBObject& DBObject::getObject(const std::string& name, int i) const
{
  FieldObjectList::const_iterator it = m_obj_v_m.find(name);
  if (it != m_obj_v_m.end()) return it->second[i];
  else throw (std::out_of_range(StringUtil::form("%s not found in %s (%s:%d)",
                                                   name.c_str(), getName().c_str(),
                                                   __FILE__, __LINE__)));
}

void DBObject::reset()
{
  const DBField::NameList& name_v(getFieldNames());
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

void DBObject::readObject(Reader& reader)
{
  reset();
  setPath(reader.readString());
  setName(reader.readString());
  setDate(reader.readInt());
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

void DBObject::writeObject(Writer& writer) const
{
  writer.writeString(getPath());
  writer.writeString(getName());
  writer.writeInt(getDate());
  const DBField::NameList& name_v(getFieldNames());
  writer.writeInt(name_v.size());
  for (DBField::NameList::const_iterator iname = name_v.begin();
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

const void* DBObject::getValue(const std::string& name) const
{
  if (!hasValue(name)) return NULL;
  FieldValueList::const_iterator it = m_value_m.find(name);
  if (it != m_value_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("value %s not found", name.c_str())));
}

const std::string& DBObject::getText(const std::string& name) const
{
  if (!hasText(name)) return m_empty;
  FieldTextList::const_iterator it = m_text_m.find(name);
  if (it != m_text_m.end()) return it->second;
  else throw (std::out_of_range(StringUtil::form("text %s not found", name.c_str())));
}

void DBObject::addValue(const std::string& name, const void* value,
                        DBField::Type type, int)
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
                        const void* value, int)
{
  const DBField::Property& pro(getProperty(name));
  int size = pro.getTypeSize();
  if (hasField(name) && size > 0) {
    memcpy(m_value_m[name], value, size);
  }
}

void DBObject::addText(const std::string& name,
                       const std::string& value)
{
  if (!hasField(name)) {
    add(name, DBField::Property(DBField::TEXT, 0));
    m_text_m.insert(FieldTextList::value_type(name, value));
  } else {
    m_text_m[name] = value;
  }
}

void DBObject::addObject(const std::string& name,
                         const DBObject& obj)
{
  if (!hasField(name)) {
    add(name, DBField::Property(DBField::OBJECT, 0));
    m_obj_v_m.insert(FieldObjectList::value_type(name, DBObjectList()));
  }
  m_obj_v_m[name].push_back(obj);
  getProperty(name).setLength(m_obj_v_m[name].size());
}

void DBObject::addObjects(const std::string& name,
                          const DBObjectList& obj_v)
{
  if (!hasField(name)) {
    add(name, DBField::Property(DBField::OBJECT, obj_v.size()));
    m_obj_v_m.insert(FieldObjectList::value_type(name, obj_v));
  } else {
    m_obj_v_m[name] = obj_v;
  }
}

void DBObject::print(bool isfull) const
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
  printf("# DB object (confname = %s) stored at %s\n", getName().c_str(), Date(getDate()).toString());
  printf("#\n");
  printf("\n");
  StringList s = StringUtil::split(getName(), '@');
  if (s.size() > 1) {
    printf("%*s : %s\n", int(-length), "nodename", s[0].c_str());
    printf("%*s : %s\n", int(-length), "config", s[1].c_str());
  } else {
    printf("%*s : %s\n", int(-length), "config", getName().c_str());
  }
  printf("\n");
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    printf("%*s : %s\n", int(-length), it->name.c_str(), it->value.c_str());
  }
  printf("\n");
  printf("#\n");
  printf("#\n");
  printf("#\n");
}

const std::string DBObject::sprint(bool isfull) const
{
  std::stringstream ss;
  const std::string& name_in = "";
  NameValueList map;
  search(map, name_in, isfull);
  size_t length = 0;
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    if (it->name.size() > length) length = it->name.size();
  }
  ss << "#" << std::endl;
  ss << "# DB object (confname = " << getName() << ")" << std::endl;
  ss << "#" << std::endl;
  ss << "" << std::endl;
  StringList s = StringUtil::split(getName(), '@');
  if (s.size() > 1) {
    ss << StringUtil::form("%*s : %s\n", -length, "nodename", s[0].c_str())
       << std:: endl;
    ss << StringUtil::form("%*s : %s\n", -length, "config", s[1].c_str())
       << std:: endl;
  } else {
    ss << StringUtil::form("%*s : %s\n", -length, "config", getName().c_str())
       << std:: endl;
  }
  ss << "" << std::endl;
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    ss << StringUtil::form("%*s : %s\n", -length, it->name.c_str(), it->value.c_str())
       << std::endl;
  }
  ss << "" << std::endl;
  ss << "#" << std::endl;
  ss << "#" << std::endl;
  ss << "#" << std::endl;
  return ss.str();
}

void DBObject::printHTML(bool isfull) const
{
  const std::string& name_in = "";
  NameValueList map;
  search(map, name_in, isfull);
  printf("<table>\n");
  printf("<caption><strong>%s</strong></caption>\n", getName().c_str());
  printf("<thead><tr><th>Name</th><th>Value</th>\n</tr></thead>\n");
  printf("<tbody>\n");
  StringList s = StringUtil::split(getName(), '@');
  if (s.size() > 1) {
    printf("<tr><td>nodename</td><td>%s</td>\n", s[0].c_str());
    printf("<tr><td>config</td><td>%s</td>\n", s[1].c_str());
  } else {
    printf("<tr><td>config</td><td>%s</td>\n", getName().c_str());
  }
  std::string rcconfig, dbtable, nodename;
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    if (it->name.find("name") != std::string::npos) {
      nodename = it->value;
      printf("<tr><td>%s</td><td>%s</td>\n", it->name.c_str(), it->value.c_str());
    } else if (it->name.find("rcconfig") != std::string::npos) {
      rcconfig = it->value;
    } else if (it->name.find("dbtable") != std::string::npos) {
      dbtable = it->value;
      printf("<tr><td>%s</td><td><a href=\"./daqconfig.php?db=%s&config=%s@%s\" >%s/%s</a></td>\n",
             it->name.c_str(), dbtable.c_str(), nodename.c_str(), rcconfig.c_str(),
             dbtable.c_str(), rcconfig.c_str());
    } else {
      printf("<tr><td>%s</td><td>%s</td>\n", it->name.c_str(), it->value.c_str());
    }
  }
  printf("</tbody>\n");
  printf("</table>\n");
}

StringList DBObject::getNameList(bool isfull) const
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
const
{
  const DBField::NameList& name_v(getFieldNames());
  for (DBField::NameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const DBField::Property& pro(getProperty(name));
    std::string name_out = name_in;
    if (name_in.size() > 0) name_out += ".";
    name_out += name;
    std::string ptype;
    switch (pro.getType()) {
      case DBField::BOOL: ptype = "bool"; break;
      case DBField::INT:  ptype = "int"; break;
      case DBField::FLOAT:  ptype = "float"; break;
      case DBField::DOUBLE:  ptype = "double"; break;
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

const std::string DBObject::printSQL(const std::string& table, int id) const
{
  std::stringstream ss;

  const std::string& name_in = "";
  NameValueList map;
  bool isfull = false;
  search(map, name_in, isfull);
  for (NameValueList::iterator it = map.begin();
       it != map.end(); it++) {
    NameValue& nv(*it);
    std::string ptype;
    std::string value = nv.value;
    std::string::size_type pos = value.find_first_of("(");
    DBField::Type type = DBField::TEXT;
    if (pos != std::string::npos) {
      std::string type_s = value.substr(0, pos);
      if (type_s == "bool") type = DBField::BOOL;
      else if (type_s == "char") type = DBField::CHAR;
      else if (type_s == "short") type = DBField::SHORT;
      else if (type_s == "int") type = DBField::INT;
      else if (type_s == "float") type = DBField::FLOAT;
      else if (type_s == "double") type = DBField::DOUBLE;
      else if (type_s == "object") type = DBField::OBJECT;
      if (type != DBField::TEXT) {
        value = StringUtil::replace(value.substr(pos + 1), ")", "");
      }
    }

    switch (nv.type) {
      case DBField::BOOL:
        ss << "insert into " << table << " (pid,name,value_b) values "
           << "(" << id << ",'" << nv.name << "'," << value << ");" << std::endl;
        break;
      case DBField::INT:
        ss << "insert into " << table << " (pid,name,value_i) values "
           << "(" << id << ",'" << nv.name << "'," << value << ");" << std::endl;
        break;
      case DBField::FLOAT:
      case DBField::DOUBLE:
        ss << "insert into " << table << " (pid,name,value_f) values "
           << "(" << id << ",'" << nv.name << "'," << value << ");" << std::endl;
        break;
      case DBField::TEXT:
        ss << "insert into " << table << " (pid,name,value_t) values "
           << "(" << id << ",'" << nv.name << "','" << value << "');" << std::endl;
        break;
      default : break;
    }
  }
  return ss.str();
}

