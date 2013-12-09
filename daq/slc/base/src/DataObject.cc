#include "daq/slc/base/DataObject.h"
#include "daq/slc/base/ConfigFile.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Reader.h"
#include "daq/slc/base/Writer.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

DataObject::DataObject()
{
  _revision = 0;
  _confno = 0;
  _id = 0;
}

DataObject::DataObject(const std::string& data_class,
                       const std::string& base_class)
{
  _revision = 0;
  _confno = 0;
  _id = 0;
  _class = data_class;
  _base_class = base_class;
}

DataObject::DataObject(DataObject* obj)
{
  _revision = obj->_revision;
  _confno = obj->_confno;
  _id = obj->_id;
  _class = obj->_class;
  _base_class = obj->_base_class;
  for (ParamNameList::iterator it = obj->_name_v.begin();
       it != obj->_name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(obj->_param_m[name]);
    if (info.type == OBJECT) {
      addObject(name, (new DataObject((DataObject*)info.buf)));
    } else {
      add(name, info.buf, info.type, info.length);
    }
  }
}

DataObject::~DataObject() throw()
{
  for (ParamNameList::iterator it = _name_v.begin();
       it != _name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(_param_m[name]);
    switch (info.type) {
      case OBJECT: delete(DataObject*)info.buf; break;
      default: free(info.buf); break;
    }
  }
}

void DataObject::print()
{
  std::cout << "-------------------" << std::endl;
  std::cout << _class << " rev = " << _revision << std::endl;
  std::cout << " config_no = " << _confno << std::endl;
  for (ParamNameList::iterator it = _name_v.begin();
       it != _name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(_param_m[name]);
    ParamType type = info.type;
    if (type == TEXT) {
      std::cout << name << " : '" << getText(name) << "'" << std::endl;
    } else if (type == OBJECT) {
      getObject(name)->print();
    } else {
      void* buf = info.buf;
      size_t length = info.length;
      if (length == 0) length = 1;
      for (size_t i = 0; i < length; i++) {
        std::cout << name;
        if (info.length > 0) std::cout << "[" << i << "] : ";
        else  std::cout << " : ";
        switch (type) {
          case BOOL:   std::cout << (((bool*)buf)[i] ? "true" : "false") << std::endl; break;
          case LONG:   std::cout << ((long long*)buf)[i] << std::endl; break;
          case ENUM:
          case INT:    std::cout << ((int*)buf)[i] << std::endl; break;
          case SHORT:  std::cout << ((short*)buf)[i] << std::endl; break;
          case CHAR:   std::cout << ((char*)buf)[i] << std::endl; break;
          case ULONG:  std::cout << ((unsigned long long*)buf)[i] << std::endl; break;
          case UINT:   std::cout << ((unsigned int*)buf)[i] << std::endl; break;
          case USHORT: std::cout << ((unsigned short*)buf)[i] << std::endl; break;
          case UCHAR:  std::cout << ((unsigned char*)buf)[i] << std::endl; break;
          case FLOAT:  std::cout << ((float*)buf)[i] << std::endl; break;
          case DOUBLE: std::cout << ((double*)buf)[i] << std::endl; break;
          default : break;
        }
      }
    }
  }
  std::cout << "-------------------" << std::endl;
}

const std::string DataObject::toSQLConfig()
{
  std::stringstream ss;
  //ss << "confno int unsigned, id smallint";
  ss << "confno int, id smallint";
  for (ParamNameList::iterator it = _name_v.begin();
       it != _name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(_param_m[name]);
    ParamType type = info.type;
    if (type == TEXT) {
      ss << ", " << name << " text";
    } else if (type != OBJECT) {
      size_t length = info.length;
      if (length == 0) length = 1;
      for (size_t i = 0; i < length; i++) {
        if (info.length > 0)
          //          ss << ", `" << name << ":" << i << "`";
          //else
          //ss << ", `" << name << "`";
          ss << ", \"" << name << ":" << i << "\"";
        else
          ss << ", \"" << name << "\"";
        switch (type) {
          case BOOL:   ss << " boolean"; break;
          case LONG:   ss << " bigint"; break;
          case ENUM:
          case INT:    ss << " int"; break;
          case SHORT:  ss << " smallint"; break;
          case CHAR:   ss << " tinyint"; break;
          case ULONG:  ss << " bigint"; break;
          case UINT:   ss << " int"; break;
          case USHORT: ss << " smallint"; break;
          case UCHAR:  ss << " tinyint"; break;
            //case ULONG:  ss << " bigint unsigned"; break;
            //case UINT:   ss << " int unsigned"; break;
            //case USHORT: ss << " smallint unsigned"; break;
            //case UCHAR:  ss << " tinyint unsigned"; break;
          case FLOAT:  ss << " float"; break;
          case DOUBLE: ss << " double"; break;
          default : break;
        }
      }
    }
  }
  return ss.str();
}

const std::string DataObject::toSQLNames()
{
  std::stringstream ss;
  ss << "confno, id";
  for (ParamNameList::iterator it = _name_v.begin();
       it != _name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(_param_m[name]);
    ParamType type = info.type;
    if (type == TEXT) {
      ss << ", " << name << "";
    } else if (type != OBJECT) {
      size_t length = info.length;
      if (length == 0) length = 1;
      for (size_t i = 0; i < length; i++) {
        if (info.length > 0)
          //          ss << ", `" << name << ":" << i << "`";
          //        else
          //          ss << ", `" << name << "`";
          ss << ", \"" << name << ":" << i << "\"";
        else
          ss << ", \"" << name << "\"";
      }
    }
  }
  return ss.str();
}

const std::string DataObject::toSQLValues()
{
  std::stringstream ss;
  ss << _confno << ", " << _id << "";
  for (ParamNameList::iterator it = _name_v.begin();
       it != _name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(_param_m[name]);
    ParamType type = info.type;
    if (type == TEXT) {
      ss << ", '" << getText(name) << "'";
    } else if (type != OBJECT) {
      void* buf = info.buf;
      size_t length = info.length;
      if (length == 0) length = 1;
      for (size_t i = 0; i < length; i++) {
        ss << ", ";
        switch (type) {
          case BOOL:   ss << (((bool*)buf)[i] ? "true" : "false"); break;
          case LONG:   ss << ((long long*)buf)[i]; break;
          case ENUM:
          case INT:    ss << ((int*)buf)[i]; break;
          case SHORT:  ss << ((short*)buf)[i]; break;
          case CHAR:   ss << ((char*)buf)[i]; break;
          case ULONG:  ss << ((unsigned long long*)buf)[i]; break;
          case UINT:   ss << ((unsigned int*)buf)[i]; break;
          case USHORT: ss << ((unsigned short*)buf)[i]; break;
          case UCHAR:  ss << ((unsigned char*)buf)[i]; break;
          case FLOAT:  ss << ((float*)buf)[i]; break;
          case DOUBLE: ss << ((double*)buf)[i]; break;
          default : break;
        }
      }
    }
  }
  return ss.str();
}

void DataObject::readObject(Reader& reader) throw(IOException)
{
  _revision = reader.readInt();
  _confno = reader.readInt();
  _id = reader.readInt();
  //_name = reader.readString();
  _class = reader.readString();
  while (true) {
    std::string name = reader.readString();
    if (name == "==OBJECT_END==") break;
    ParamInfo info;
    info.type = (ParamType)reader.readInt();
    info.length = reader.readUInt();
    if (!hasValue(name)) {
      info.buf = NULL;
    } else {
      info = _param_m[name];
    }
    if (info.type == TEXT) {
      std::string str = reader.readString();
      if (info.buf == NULL) addText(name, str, info.length);
      else setText(name, str);
    } else if (info.type == OBJECT) {
      if (info.buf == NULL) addObject(name, new DataObject());
      getObject(name)->readObject(reader);
    } else {
      if (info.type == ENUM) {
        EnumMap enum_m;
        while (true) {
          std::string label = reader.readString();
          if (label == "==ENUM_END==") break;
          enum_m.insert(EnumMap::value_type(label, reader.readInt()));
        }
        if (_enum_m_m.find(name) == _enum_m_m.end())
          _enum_m_m.insert(EnumMapMap::value_type(name, enum_m));
      }
      if (info.buf == NULL) {
        add(name, NULL, info.type, info.length);
      }
      info = _param_m[name];
      void* buf = info.buf;
      size_t length = info.length;
      if (length == 0) length = 1;
      for (size_t i = 0; i < length; i++) {
        switch (info.type) {
          case BOOL: ((bool*)buf)[i] = reader.readBool(); break;
          case LONG: ((long long*)buf)[i] = reader.readLong(); break;
          case ENUM:
          case INT: ((int*)buf)[i] = reader.readInt(); break;
          case SHORT: ((short*)buf)[i] = reader.readShort(); break;
          case CHAR: ((char*)buf)[i] = reader.readChar(); break;
          case ULONG: ((unsigned long long*)buf)[i] = reader.readULong(); break;
          case UINT: ((unsigned int*)buf)[i] = reader.readUInt(); break;
          case USHORT: ((unsigned short*)buf)[i] = reader.readUShort(); break;
          case UCHAR: ((unsigned char*)buf)[i] = reader.readUChar(); break;
          case FLOAT: ((float*)buf)[i] = reader.readFloat(); break;
          case DOUBLE: ((double*)buf)[i] = reader.readDouble(); break;
          default : break;
        }
      }
    }
  }
}

void DataObject::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt(_revision);
  writer.writeInt(_confno);
  writer.writeInt(_id);
  //writer.writeString(_name);
  writer.writeString(_class);
  for (ParamNameList::iterator it = _name_v.begin();
       it != _name_v.end(); it++) {
    std::string& name(*it);
    ParamInfo& info(_param_m[name]);
    writer.writeString(name);
    writer.writeInt(info.type);
    writer.writeUInt(info.length);
    if (info.type == TEXT) {
      writer.writeString(getText(name));
    } else if (info.type == OBJECT) {
      writer.writeObject(*(const DataObject*)info.buf);
    } else {
      void* buf = info.buf;
      size_t length = info.length;
      if (length == 0) length = 1;
      for (size_t i = 0; i < length; i++) {
        switch (info.type) {
          case BOOL:   writer.writeBool(((bool*)buf)[i]); break;
          case LONG:   writer.writeLong(((long long*)buf)[i]); break;
          case ENUM: {
            EnumMap& enum_m(_enum_m_m[name]);
            for (EnumMap::iterator iit = enum_m.begin(); iit != enum_m.end(); iit++) {
              writer.writeString(iit->first);
              writer.writeInt(iit->second);
            }
            writer.writeString("==ENUM_END==");
          }
          case INT:    writer.writeInt(((int*)buf)[i]); break;
          case SHORT:  writer.writeShort(((short*)buf)[i]); break;
          case CHAR:   writer.writeChar(((char*)buf)[i]); break;
          case ULONG:  writer.writeULong(((unsigned long long*)buf)[i]); break;
          case UINT:   writer.writeUInt(((unsigned int*)buf)[i]); break;
          case USHORT: writer.writeUShort(((unsigned short*)buf)[i]); break;
          case UCHAR:  writer.writeUChar(((unsigned char*)buf)[i]); break;
          case FLOAT:  writer.writeFloat(((float*)buf)[i]); break;
          case DOUBLE: writer.writeDouble(((double*)buf)[i]); break;
          default : break;
        }
      }
    }
  }
  writer.writeString("==OBJECT_END==");
}

void DataObject::setValues(std::vector<std::string>& name_v,
                           std::vector<std::string>& value_v)
{
  for (size_t i = 0; i < name_v.size(); i++) {
    setValue(name_v[i], value_v[i]);
  }
}

void DataObject::setValue(const std::string& name_in, const std::string& value)
{
  std::vector<std::string> str_v = Belle2::split(name_in, ':');
  std::string name = str_v[0];
  if (_param_m.find(name) != _param_m.end()) {
    ParamInfo& info(_param_m[name]);
    ParamType type = info.type;
    void* buf = info.buf;
    if (type == TEXT) {
      setText(name, value);
    } else if (type != OBJECT) {
      size_t i = (str_v.size() > 1) ? atoi(str_v[1].c_str()) : 0;
      switch (type) {
        case BOOL: ((bool*)buf)[i] = (value == "true" || value == "t" || value == "1"); break;
        case LONG: ((long long*)buf)[i] = atol(value.c_str()); break;
        case ENUM: ((int*)buf)[i] = _enum_m_m[name][value]; break;
        case INT: ((int*)buf)[i] = atoi(value.c_str()); break;
        case SHORT: ((short*)buf)[i] = (short)atoi(value.c_str()); break;
        case CHAR: ((char*)buf)[i] = (char)atoi(value.c_str()); break;
        case ULONG: ((unsigned long long*)buf)[i] = strtoul(value.c_str(), 0, 0); break;
        case UINT: ((unsigned int*)buf)[i] = (unsigned int)strtoul(value.c_str(), 0, 0);  break;
        case USHORT: ((unsigned short*)buf)[i] = (unsigned short)strtoul(value.c_str(), 0, 0);  break;
        case UCHAR: ((unsigned char*)buf)[i] = (unsigned char)strtoul(value.c_str(), 0, 0);  break;
        case DOUBLE: ((double*)buf)[i] = atof(value.c_str()); break;
        case FLOAT: ((float*)buf)[i] = (float)atof(value.c_str()); break;
        default : break;
      }
    }
  }
}

void DataObject::setValue(const std::string& name, const std::string& value_in, int length)
{
  if (_param_m.find(name) != _param_m.end()) {
    ParamInfo& info(_param_m[name]);
    ParamType type = info.type;
    if (type == TEXT) {
      setText(name, value_in);
    } else if (type != OBJECT) {
      if (length == 0) length = 1;
      std::vector<std::string> value_v = Belle2::split(value_in, ',');
      if (length > (int)value_v.size()) {
        value_v = std::vector<std::string>();
        for (int i = 0; i < length; i++) value_v.push_back(value_in);
      }
      for (int i = 0; i < length; i++) {
        std::string value = value_v[i];
        void* buf = info.buf;
        switch (type) {
          case BOOL: ((bool*)buf)[i] = (value == "true" || value == "t" || value == "1"); break;
          case LONG: ((long long*)buf)[i] = atol(value.c_str()); break;
          case ENUM: ((int*)buf)[i] = _enum_m_m[name][value]; break;
          case INT: ((int*)buf)[i] = atoi(value.c_str()); break;
          case SHORT: ((short*)buf)[i] = (short)atoi(value.c_str()); break;
          case CHAR: ((char*)buf)[i] = (char)atoi(value.c_str()); break;
          case ULONG: ((unsigned long long*)buf)[i] = strtoul(value.c_str(), 0, 0); break;
          case UINT: ((unsigned int*)buf)[i] = (unsigned int)strtoul(value.c_str(), 0, 0);  break;
          case USHORT: ((unsigned short*)buf)[i] = (unsigned short)strtoul(value.c_str(), 0, 0);  break;
          case UCHAR: ((unsigned char*)buf)[i] = (unsigned char)strtoul(value.c_str(), 0, 0);  break;
          case DOUBLE: ((double*)buf)[i] = atof(value.c_str()); break;
          case FLOAT: ((float*)buf)[i] = (float)atof(value.c_str()); break;
          default : break;
        }
      }
    }
  }
}

bool DataObject::hasValue(const std::string& name)
{
  return _param_m.find(name) != _param_m.end();
}

bool DataObject::hasObject(const std::string& name)
{
  return (_param_m.find(name) != _param_m.end() &&
          _param_m[name].type == OBJECT);
}

bool DataObject::hasInt(const std::string& name)
{
  return (_param_m.find(name) != _param_m.end() &&
          _param_m[name].type == INT);
}

void DataObject::addEnum(const std::string& name, EnumMap& enum_m,
                         const std::string& value)
{
  _enum_m_m.insert(EnumMapMap::value_type(name, enum_m));
  add(name, &enum_m[value], ENUM, 0);
}

void DataObject::addEnumArray(const std::string& name, EnumMap& enum_m,
                              const std::string& value, size_t length)
{
  _enum_m_m.insert(EnumMapMap::value_type(name, enum_m));
  add(name, NULL, ENUM, length);
  setValue(name, value);
}

void DataObject::addText(const std::string& name, const std::string& value, size_t length)
{
  addText(name, value.c_str(), length);
}

void DataObject::addText(const std::string& name, const char* value, size_t length)
{
  ParamInfo info = { TEXT, length, malloc(length) };
  strcpy((char*)info.buf, value);
  _param_m.insert(ParamInfoMap::value_type(name, info));
  _name_v.push_back(name);
}

void DataObject::addObject(const std::string& name, DataObject* value)
{
  if (value == NULL) return;
  ParamInfo info = { OBJECT, 0, value };
  _param_m.insert(ParamInfoMap::value_type(name, info));
  _name_v.push_back(name);
}

void DataObject::add(const std::string& name, void* value,
                     ParamType type, size_t length)
{
  ParamInfo info = { type, length, NULL };
  size_t size = 0;
  if (length == 0) length = 1;
  switch (type) {
    case BOOL:   size = sizeof(bool) * length; break;
    case CHAR:   size = sizeof(char) * length; break;
    case TEXT:   size = sizeof(char) * length; break;
    case UCHAR:  size = sizeof(unsigned char) * length; break;
    case ENUM:
    case INT:    size = sizeof(int) * length; break;
    case UINT:   size = sizeof(unsigned int) * length; break;
    case FLOAT:  size = sizeof(float) * length; break;
    case SHORT:  size = sizeof(unsigned short) * length; break;
    case USHORT: size = sizeof(short) * length; break;
    case LONG:   size = sizeof(long long) * length; break;
    case ULONG:  size = sizeof(unsigned long long) * length; break;
    case DOUBLE: size = sizeof(double) * length; break;
    default : break;
  }
  if (size > 0) {
    info.buf = malloc(size);
    memset(info.buf, 0, size);
    if (info.buf != NULL && value != NULL) memcpy(info.buf, value, size);
    _param_m.insert(ParamInfoMap::value_type(name, info));
    _name_v.push_back(name);
  }
}

void DataObject::set(const std::string& name, void* value, size_t size)
{
  memcpy(_param_m[name].buf, value, size);
}

void DataObject::setText(const std::string& name, const std::string& value)
{
  setText(name, value.c_str());
}
void DataObject::setText(const std::string& name, const char* value)
{
  strcpy((char*)_param_m[name].buf, value);
}
