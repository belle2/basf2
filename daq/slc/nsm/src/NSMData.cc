#include "daq/slc/nsm/NSMData.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/Writer.h"
#include "daq/slc/base/Reader.h"

#include "nsm2/belle2nsm.h"
extern "C" {
#include "nsm2/nsmlib2.h"
#if NSM_PACKAGE_VERSION >= 1914
#include "nsm2/nsmparse.h"
#endif
}

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

void* NSMData::open(NSMCommunicator* comm) throw(NSMHandlerException)
{
  b2nsm_context(comm->getContext());
  if ((_pdata = b2nsm_openmem(_data_name.c_str(), _format.c_str(), _revision))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to open data memory"));
  }
  parse(NULL, false);
  return _pdata;
}

void* NSMData::allocate(NSMCommunicator* comm, int interval) throw(NSMHandlerException)
{
  b2nsm_context(comm->getContext());
  if ((_pdata = b2nsm_allocmem(_data_name.c_str(), _format.c_str(), _revision, interval))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to allocate data memory"));
  }
  parse(NULL, false);
  return _pdata;
}

void* NSMData::parse(const char* incpath, bool maclloc_new) throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  NSMparse* ptr = NULL;
  char fmtstr[256];
  if ((ptr = nsmlib_parsefile(_format.c_str(), _revision, incpath, fmtstr)) == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to parse header file"));
  }
  int size = 0;
  while (ptr != NULL) {
    NSMDataProperty pro;
    int length = ptr->size;
    char type = ptr->type;
    std::string label = ptr->name;
    pro.offset = ptr->offset;
    if (type == 'l') pro.type = INT64;
    else if (type == 'i') pro.type = INT32;
    else if (type == 's') pro.type = INT16;
    else if (type == 'c') pro.type = CHAR;
    else if (type == 'L') pro.type = UINT64;
    else if (type == 'I') pro.type = UINT32;
    else if (type == 'S') pro.type = UINT16;
    else if (type == 'C') pro.type = BYTE8;
    else if (type == 'd') pro.type = DOUBLE;
    else if (type == 'f') pro.type = FLOAT;
    size += (length < 0) ? pro.type % 100 : (pro.type % 100) * length;
    pro.length = length;
    Belle2::debug("%s %d %c %d", ptr->name, size, ptr->type, ptr->offset);
    _pro_m.insert(NSMDataPropertyMap::value_type(label, pro));
    _label_v.push_back(label);
    ptr = ptr->next;
  }
  if (size > 0) return malloc(size);
#endif
  return NULL;
}

void* NSMData::get() throw(NSMHandlerException)
{
  if (_pdata == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM data is not ready."));
  }
  return _pdata;
}

const void* NSMData::get() const throw(NSMHandlerException)
{
  if (_pdata == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "NSM data is not ready."));
  }
  return _pdata;
}

void* NSMData::getValue(const std::string& label) throw(NSMHandlerException)
{
  char* data = (char*)get();
  return (data + _pro_m[label].offset);
}

void* NSMData::getValueArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  char* data = (char*)get();
  length = _pro_m[label].length;
  return (data + _pro_m[label].offset);
}

void NSMData::setValue(const std::string& label,
                       const void* value, size_t length) throw(NSMHandlerException)
{
  char* data = (char*)get();
  memcpy(data + _pro_m[label].offset, value, length);
}

int64 NSMData::getInt64(const std::string& label) throw(NSMHandlerException)
{
  return *(int64*)getValue(label);
}

int32 NSMData::getInt32(const std::string& label) throw(NSMHandlerException)
{
  return *(int32*)getValue(label);
}

int16 NSMData::getInt16(const std::string& label) throw(NSMHandlerException)
{
  return *(int16*)getValue(label);
}

char NSMData::getChar(const std::string& label) throw(NSMHandlerException)
{
  return *(char*)getValue(label);
}

uint64 NSMData::getUInt64(const std::string& label) throw(NSMHandlerException)
{
  return *(uint64*)getValue(label);
}

uint32 NSMData::getUInt32(const std::string& label) throw(NSMHandlerException)
{
  return *(uint32*)getValue(label);
}

uint16 NSMData::getUInt16(const std::string& label) throw(NSMHandlerException)
{
  return *(uint16*)getValue(label);
}

byte8 NSMData::getByte(const std::string& label) throw(NSMHandlerException)
{
  return *(byte8*)getValue(label);
}

double NSMData::getDouble(const std::string& label) throw(NSMHandlerException)
{
  return *(double*)getValue(label);
}

float NSMData::getFloat(const std::string& label) throw(NSMHandlerException)
{
  return *(float*)getValue(label);
}

int64* NSMData::getInt64Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (int64*)getValueArray(label, length);
}

int32* NSMData::getInt32Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (int32*)getValueArray(label, length);
}

int16* NSMData::getInt16Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (int16*)getValueArray(label, length);
}

char* NSMData::getCharArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (char*)getValueArray(label, length);
}

uint64* NSMData::getUInt64Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (uint64*)getValueArray(label, length);
}

uint32* NSMData::getUInt32Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (uint32*)getValueArray(label, length);
}

uint16* NSMData::getUInt16Array(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (uint16*)getValueArray(label, length);
}

byte8* NSMData::getByteArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (byte8*)getValueArray(label, length);
}

double* NSMData::getDoubleArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (double*)getValueArray(label, length);
}

float* NSMData::getFloatArray(const std::string& label, size_t& length) throw(NSMHandlerException)
{
  return (float*)getValueArray(label, length);
}

void NSMData::setInt64(const std::string& label, int64 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(int64));
}

void NSMData::setInt32(const std::string& label, int32 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(int32));
}

void NSMData::setInt16(const std::string& label, int16 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(int16));
}

void NSMData::setChar(const std::string& label, char value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(char));
}

void NSMData::setUInt64(const std::string& label, uint64 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(uint64));
}

void NSMData::setUInt32(const std::string& label, uint32 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(uint32));
}

void NSMData::setUInt16(const std::string& label, uint16 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(uint16));
}

void NSMData::setByte8(const std::string& label, byte8 value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(byte8));
}

void NSMData::setDouble(const std::string& label, double value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(double));
}

void NSMData::setFloat(const std::string& label, float value) throw(NSMHandlerException)
{
  setValue(label, &value, sizeof(float));
}

const std::string NSMData::toSQLConfig()
{
  std::stringstream ss;
  ss << "record_time timestamp";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    std::string type_s;
    switch (pro.type) {
      case INT64:  type_s = "bigint"; break;
      case INT32:  type_s = "int"; break;
      case INT16:  type_s = "smallint"; break;
      case CHAR:   type_s = "tinyint"; break;
      case UINT64: type_s = "bigint unsigned"; break;
      case UINT32: type_s = "int unsigned"; break;
      case UINT16: type_s = "smallint unsigned"; break;
      case BYTE8:  type_s = "tinyint unsigned"; break;
      case DOUBLE: type_s = "double"; break;
      case FLOAT:  type_s = "float"; break;
      default : break;
    }
    for (size_t i = 0; i < pro.length; i++) {
      ss << ", `" << label << ":" << i << "` " << type_s;
    }
  }
  return ss.str();
}

const std::string NSMData::toSQLNames()
{
  std::stringstream ss;
  ss << "record_time";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    if (pro.length == 0) {
      ss << ", " << label;
    } else {
      for (size_t i = 0; i < pro.length; i++) {
        ss << ", `" << label << ":" << i << "`";
      }
    }
  }
  return ss.str();
}

const std::string NSMData::toSQLValues()
{
  char* data = (char*)get();
  std::stringstream ss;
  ss << "current_timestamp";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    int length = pro.length;
    if (length < 0) length = 1;
    for (int i = 0; i < length; i++) {
      ss << ", ";
      switch (pro.type) {
        case INT64:  ss << ((int64*)(data + pro.offset))[i]; break;
        case INT32:  ss << ((int32*)(data + pro.offset))[i]; break;
        case INT16:  ss << ((int16*)(data + pro.offset))[i]; break;
        case CHAR:   ss << (int)((char*)(data + pro.offset))[i]; break;
        case UINT64: ss << ((uint64*)(data + pro.offset))[i]; break;
        case UINT32: ss << ((uint32*)(data + pro.offset))[i]; break;
        case UINT16: ss << ((uint16*)(data + pro.offset))[i]; break;
        case BYTE8:  ss << (uint32)((byte8*)(data + pro.offset))[i]; break;
        case DOUBLE: ss << ((double*)(data + pro.offset))[i]; break;
        case FLOAT:  ss << ((float*)(data + pro.offset))[i]; break;
        default : break;
      }
    }
  }
  return ss.str();
}

void NSMData::setSQLValues(std::vector<std::string>& name_v,
                           std::vector<std::string>& value_v)
{
  char* data = (char*)get();
  for (size_t i = 0; i < name_v.size(); i++) {
    std::string name = name_v[i];
    std::vector<std::string> str_v = Belle2::split(name, ':');
    int index = 0;
    if (str_v.size() > 1) {
      name = str_v[0];
      index = atoi(str_v[1].c_str());
    }
    NSMDataProperty& pro(_pro_m[name]);
    switch (pro.type) {
      case INT64: ((int64*)(data + pro.offset))[index] = atoll(value_v[i].c_str()); break;
      case INT32: ((int32*)(data + pro.offset))[index] = atoi(value_v[i].c_str()); break;
      case INT16: ((int16*)(data + pro.offset))[index] = atoi(value_v[i].c_str()); break;
      case CHAR: ((char*)(data + pro.offset))[index] = (char)atoi(value_v[i].c_str()); break;
      case UINT64: ((uint64*)(data + pro.offset))[index] = strtoll(value_v[i].c_str(), NULL, 0); break;
      case UINT32: ((uint32*)(data + pro.offset))[index] = strtol(value_v[i].c_str(), NULL, 0); break;
      case UINT16: ((uint16*)(data + pro.offset))[index] = strtol(value_v[i].c_str(), NULL, 0); break;
      case BYTE8: ((byte8*)(data + pro.offset))[index] = (byte8)atoi(value_v[i].c_str()); break;
      case DOUBLE: ((double*)(data + pro.offset))[index] = atof(value_v[i].c_str()); break;
      case FLOAT: ((float*)(data + pro.offset))[index] = atof(value_v[i].c_str()); break;
      default : break;
    }
  }
}

void NSMData::getSQLValues(std::vector<std::string>&,
                           std::vector<std::string>& value_v)
{
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    int length = pro.length;
    if (length < 0) length = 1;
    for (int i = 0; i < length; i++) {
      std::stringstream ss;
      switch (pro.type) {
        case INT64:  ss << ((int64*)(data + pro.offset))[i];  break;
        case INT32:  ss << ((int32*)(data + pro.offset))[i];  break;
        case INT16:  ss << ((int16*)(data + pro.offset))[i];  break;
        case CHAR:   ss << ((char*)(data + pro.offset))[i];   break;
        case UINT64: ss << ((uint64*)(data + pro.offset))[i]; break;
        case UINT32: ss << ((uint32*)(data + pro.offset))[i]; break;
        case UINT16: ss << ((uint16*)(data + pro.offset))[i]; break;
        case BYTE8:  ss << ((byte8*)(data + pro.offset))[i];  break;
        case DOUBLE: ss << ((double*)(data + pro.offset))[i]; break;
        case FLOAT:  ss << ((float*)(data + pro.offset))[i];  break;
        default : break;
      }
      value_v.push_back(ss.str());
    }
  }
}

const std::string NSMData::toXML()
{
  std::stringstream ss;
  ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
     << "<object class=\"" << _data_name
     << "\" revision=\"" << _revision << "\" >" << std::endl;
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    switch (pro.type) {
      case INT64:  ss << "<int name=\"" << label
                        << "\" value=\"" << *(int64*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case INT32:  ss << "<int name=\"" << label
                        << "\" value=\"" << *(int32*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case INT16:  ss << "<short name=\"" << label
                        << "\" value=\"" << *(int16*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case CHAR:   ss << "<int name=\"" << label
                        << "\" value=\"" << *(char*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case UINT64: ss << "<int name=\"" << label
                        << "\" value=\"" << *(int64*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case UINT32: ss << "<int name=\"" << label
                        << "\" value=\"" << *(int32*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case UINT16: ss << "<short name=\"" << label
                        << "\" value=\"" << *(int16*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case BYTE8:  ss << "<byte name=\"" << label
                        << "\" value=\"" << *(byte8*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case DOUBLE: ss << "<double name=\"" << label
                        << "\" value=\"" << *(double*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      case FLOAT:  ss << "<float name=\"" << label
                        << "\" value=\"" << *(float*)(data + pro.offset)
                        << "\" />" << std::endl; break;
      default : break;
    }
  }
  return ss.str();
}

void NSMData::readObject(Reader& reader) throw(IOException)
{
  /*
  DataObject* obj = new DataObject();
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    std::stringstream ss;
    DataObject::ParamInfo pinfo = { DataObject::INT, 0, NULL };
    pinfo.buf = data + pro.offset;
    pinfo.length = (pro.length <= 0) ? 0 : pro.length;
    switch (pro.type) {
      case INT64:  pinfo.type = DataObject::LONG;   break;
      case INT32:  pinfo.type = DataObject::INT;    break;
      case INT16:  pinfo.type = DataObject::SHORT;  break;
      case CHAR:   pinfo.type = DataObject::CHAR;   break;
      case UINT64: pinfo.type = DataObject::ULONG;  break;
      case UINT32: pinfo.type = DataObject::UINT;   break;
      case UINT16: pinfo.type = DataObject::USHORT; break;
      case BYTE8:  pinfo.type = DataObject::UCHAR;  break;
      case DOUBLE: pinfo.type = DataObject::DOUBLE; break;
      case FLOAT:  pinfo.type = DataObject::FLOAT;  break;
      default : break;
    }
    obj->add(label, pinfo.buf, pinfo.type, pinfo.length);
  }
  return obj;
  */
}

void NSMData::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(_data_name);
  writer.writeString(_format);
  writer.writeInt(_revision);
  writer.writeInt(_label_v.size());
  char* data = (char*)get();
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    writer.writeString(label);
    writer.writeInt(pro.type);
    writer.writeInt(pro.length);
    int length = pro.length;
    if (length < 0) length = 1;
    for (int i = 0; i < length; i++) {
      switch (pro.type) {
        case INT64:  writer.writeLong(((int64*)(data + pro.offset))[i]); break;
        case INT32:  writer.writeInt(((int32*)(data + pro.offset))[i]); break;
        case INT16:  writer.writeShort(((int16*)(data + pro.offset))[i]); break;
        case CHAR:   writer.writeChar(((char*)(data + pro.offset))[i]); break;
        case UINT64: writer.writeLong(((uint64*)(data + pro.offset))[i]); break;
        case UINT32: writer.writeInt(((uint32*)(data + pro.offset))[i]); break;
        case UINT16: writer.writeShort(((uint16*)(data + pro.offset))[i]); break;
        case BYTE8:  writer.writeChar(((byte8*)(data + pro.offset))[i]); break;
        case DOUBLE: writer.writeDouble(((double*)(data + pro.offset))[i]); break;
        case FLOAT:  writer.writeFloat(((float*)(data + pro.offset))[i]); break;
        default : break;
      }
    }
  }
}

