#include "NSMData.hh"

#include <util/StringUtil.hh>

#include <belle2nsm.h>

#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace B2DAQ;

void* NSMData::open() throw(NSMHandlerException)
{
  if ((_pdata = b2nsm_openmem(_data_name.c_str(), _format.c_str(), _revision))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to open data memory"));
  }
  initProperties();
  return _pdata;
}

void* NSMData::allocate(int interval) throw(NSMHandlerException)
{
  if ((_pdata = b2nsm_allocmem(_data_name.c_str(), _format.c_str(), _revision, interval))
      == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to allocate data memory"));
  }
  initProperties();
  return _pdata;
}

void NSMData::initProperties() throw()
{
  nsm_data_att_t* nsm_data_att_p = nsm_data_att_list;
  while (nsm_data_att_p->length != 0) {
    if (nsm_data_att_p->length < 0) {
      printf("%s %s\n", nsm_data_att_p->type,
             nsm_data_att_p->label);
    } else {
      printf("%s %s [%d]\n", nsm_data_att_p->type,
             nsm_data_att_p->label, nsm_data_att_p->length);
    }
    NSMDataProperty pro;
    int length = nsm_data_att_p->length;
    std::string type = nsm_data_att_p->type;
    std::string label = nsm_data_att_p->label;
    pro.offset = nsm_data_att_p->offset;
    nsm_data_att_p++;
    if (type == "int64") pro.type = INT64;
    else if (type == "int32") pro.type = INT32;
    else if (type == "int16") pro.type = INT16;
    else if (type == "char" && length < 0) pro.type = CHAR;
    else if (type == "char" && length > 0) pro.type = TEXT;
    else if (type == "uint64") pro.type = UINT64;
    else if (type == "uint32") pro.type = UINT32;
    else if (type == "uint16") pro.type = UINT16;
    else if (type == "byte8" || type == "uchar") pro.type = BYTE8;
    else if (type == "double") pro.type = DOUBLE;
    else if (type == "float") pro.type = FLOAT;
    if (length < 0 || pro.type == TEXT) {
      pro.length = length;
      _pro_m.insert(NSMDataPropertyMap::value_type(label, pro));
      _label_v.push_back(label);
    } else if (length > 0) {
      for (int i = 0; i < length; i++) {
        std::string label_i = B2DAQ::form("%s_%d", label.c_str(), i);
        _pro_m.insert(NSMDataPropertyMap::value_type(label_i, pro));
        _label_v.push_back(label_i);
        pro.offset += pro.type % 100;
      }
    }
  }
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

const std::string NSMData::toSQLConfig()
{
  std::stringstream ss;
  ss << "record_time timestamp";
  for (std::vector<std::string>::iterator it = _label_v.begin();
       it != _label_v.end(); it++) {
    std::string& label(*it);
    NSMDataProperty& pro(_pro_m[label]);
    ss << ", " << label;
    switch (pro.type) {
      case INT64:  ss << " bigint"; break;
      case INT32:  ss << " int"; break;
      case INT16:  ss << " smallint"; break;
      case CHAR:   ss << " tinyint"; break;
      case UINT64: ss << " bigint unsigned"; break;
      case UINT32: ss << " int unsigned"; break;
      case UINT16: ss << " smallint unsigned"; break;
      case BYTE8:  ss << " tinyint unsigned"; break;
      case DOUBLE: ss << " double"; break;
      case FLOAT:  ss << " float"; break;
      case TEXT:   ss << " text"; break;
      default : break;
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
    ss << ", " << label;
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
    ss << ", ";
    switch (pro.type) {
      case INT64:  ss << *(int64*)(data + pro.offset); break;
      case INT32:  ss << *(int32*)(data + pro.offset); break;
      case INT16:  ss << *(int16*)(data + pro.offset); break;
      case CHAR:   ss << (int)*(char*)(data + pro.offset); break;
      case UINT64: ss << *(uint64*)(data + pro.offset); break;
      case UINT32: ss << *(uint32*)(data + pro.offset); break;
      case UINT16: ss << *(uint16*)(data + pro.offset); break;
      case BYTE8:  ss << (uint32)*(byte8*)(data + pro.offset); break;
      case DOUBLE: ss << *(double*)(data + pro.offset); break;
      case FLOAT:  ss << *(float*)(data + pro.offset); break;
      case TEXT:   ss << (const char*)(data + pro.offset); break;
      default : break;
    }
  }
  return ss.str();
}

void NSMData::setSLQValues(std::vector<std::string>& name_v,
                           std::vector<std::string>& value_v)
{
  char* data = (char*)get();
  for (size_t i = 0; i < name_v.size(); i++) {
    NSMDataProperty& pro(_pro_m[name_v[i]]);
    switch (pro.type) {
      case INT64:  *(int64*)(data + pro.offset) = atoi(value_v[i].c_str()); break;
      case INT32:  *(int32*)(data + pro.offset) = atoi(value_v[i].c_str()); break;
      case INT16:  *(int16*)(data + pro.offset) = atoi(value_v[i].c_str()); break;
      case CHAR:   *(char*)(data + pro.offset) = (char)atoi(value_v[i].c_str()); break;
      case UINT64: *(uint64*)(data + pro.offset) = atoi(value_v[i].c_str()); break;
      case UINT32: *(uint32*)(data + pro.offset) = atoi(value_v[i].c_str()); break;
      case UINT16: *(uint16*)(data + pro.offset) = atoi(value_v[i].c_str()); break;
      case BYTE8:  *(byte8*)(data + pro.offset) = (byte8)atoi(value_v[i].c_str()); break;
      case DOUBLE: *(double*)(data + pro.offset) = atof(value_v[i].c_str()); break;
      case FLOAT:  *(float*)(data + pro.offset) = atof(value_v[i].c_str()); break;
      case TEXT:   strncpy((char*)(data + pro.offset), value_v[i].c_str(), pro.length); break;
      default : break;
    }
  }
}
