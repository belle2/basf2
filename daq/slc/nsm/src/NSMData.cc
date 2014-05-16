#include "daq/slc/nsm/NSMData.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>

#include <nsm2/belle2nsm.h>
extern "C" {
#include <nsm2/nsmlib2.h>
}

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace Belle2;

NSMData::NSMData(const std::string& dataname,
                 const std::string& format, int revision)
throw() : LoggerObject(), m_allocated(false),
  m_pdata(NULL), m_size(0), m_offset(0)
{
  setName(dataname);
  setFormat(format);
  setRevision(revision);
  setConfig(false);
}

NSMData::NSMData()
throw() : LoggerObject(), m_allocated(false),
  m_pdata(NULL), m_size(0), m_offset(0)
{
  setConfig(false);
}

NSMData::NSMData(const NSMData& data) throw()
  : LoggerObject(data)
{
  setConfig(false);
  m_allocated = data.m_allocated;
  m_size = data.m_size;
  setFormat(data.getFormat());
  setRevision(data.getRevision());
  setNode(data.getNode());
  setIndex(data.getIndex());
  setName(data.getName());
  m_pdata = NULL;
  if (m_allocated) {
    m_pdata = malloc(m_size);
    memcpy(m_pdata, data.m_pdata, m_size);
  } else {
    if (data.m_pdata != NULL) m_pdata = data.m_pdata;
  }
  const FieldNameList& name_v(data.getFieldNames());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    const FieldInfo::Property& pro(data.getProperty(name));
    add(name, pro);
    if (pro.getType() == FieldInfo::NSM_OBJECT) {
      NSMDataList data_v(data.getObjects(name));
      for (size_t i = 0; i < data_v.size(); i++) {
        NSMData& cdata(data_v[i]);
        cdata.m_pdata = (void*)((char*)m_pdata +
                                pro.getOffset() + cdata.m_size * i);
      }
      m_data_v_m.insert(NSMDataListMap::value_type(name, data_v));
    }
  }
}

NSMData::~NSMData() throw()
{
  reset();
  if (m_allocated && m_pdata != NULL) {
    free(m_pdata);
    m_pdata = NULL;
  }
}

void NSMData::reset() throw()
{
  DBObject::reset();
  if (m_allocated && m_pdata != NULL) {
    free(m_pdata);
    m_pdata = NULL;
  }
}

void NSMData::addValue(const std::string& name, const void* data,
                       FieldInfo::Type type, int length) throw()
{
  FieldInfo::Property pro(type, length, m_offset);
  if (length == 0) length = 1;
  int size = pro.getTypeSize() * length;
  if (size <= 0) return;
  if (!hasField(name)) {
    add(name, pro);
    m_offset += size;
  }
  if (data != NULL) {
    memcpy((char*)m_pdata + pro.getOffset(), data, size);
  }
}

void NSMData::setValue(const std::string& name, const void* data,
                       int length) throw()
{
  const FieldInfo::Property& pro(getProperty(name));
  if (length == 0) length = 1;
  int size = pro.getTypeSize() * length;
  if (data != NULL && hasField(name) && size > 0) {
    memcpy((char*)m_pdata + pro.getOffset(), data, size);
  }
}

void* NSMData::open(NSMCommunicator* comm)
throw(NSMHandlerException)
{
  b2nsm_context(comm->getContext());
  if ((m_pdata = b2nsm_openmem(getName().c_str(), getFormat().c_str(),
                               getRevision())) == NULL) {
    throw (NSMHandlerException("Failed to open data memory %s",
                               nsmlib_strerror(comm->getContext())));
  }
  setNode(comm->getNode().getName());
  parse();
  return m_pdata;
}

void* NSMData::allocate(NSMCommunicator* comm, int interval)
throw(NSMHandlerException)
{
  b2nsm_context(comm->getContext());
  if ((m_pdata = b2nsm_allocmem(getName().c_str(), getFormat().c_str(),
                                getRevision(), interval)) == NULL) {
    throw (NSMHandlerException("Failed to allocate data memory"));
  }
  setNode(comm->getNode().getName());
  parse();
  memset(m_pdata, 0, m_size);
  return m_pdata;
}

void* NSMData::parse(const char* incpath, bool malloc_new)
throw(NSMHandlerException)
{
#if NSM_PACKAGE_VERSION >= 1914
  NSMparse* ptr = NULL;
  char fmtstr[256];
  if ((ptr = nsmlib_parsefile(getFormat().c_str(),
                              getRevision(), incpath, fmtstr)) == NULL) {
    throw (NSMHandlerException("Failed to parse header file"));
  }
  int length;
  parse(ptr, length);
  if (m_size > 0 && malloc_new) {
    m_allocated = true;
    return malloc(m_size);
  }
#endif
  return NULL;
}

#if NSM_PACKAGE_VERSION >= 1914
NSMparse* NSMData::parse(NSMparse* ptr, int& length) throw(NSMHandlerException)
{
  m_size = 0;
  while (ptr != NULL) {
    length = ptr->size;
    if (length < 0) length = 0;
    int type = ptr->type;
    std::string name = ptr->name;
    int offset = ptr->offset;
    m_offset = offset;
    if (type == 'l') type = FieldInfo::NSM_INT64;
    else if (type == 'i') type = FieldInfo::NSM_INT32;
    else if (type == 's') type = FieldInfo::NSM_INT16;
    else if (type == 'c') type = FieldInfo::NSM_CHAR;
    else if (type == 'L') type = FieldInfo::NSM_UINT64;
    else if (type == 'I') type = FieldInfo::NSM_UINT32;
    else if (type == 'S') type = FieldInfo::NSM_UINT16;
    else if (type == 'C') type = FieldInfo::NSM_BYTE8;
    else if (type == 'd') type = FieldInfo::NSM_DOUBLE;
    else if (type == 'f') type = FieldInfo::NSM_FLOAT;
    else if (type == '(') {
      NSMData data(getName(), getFormat() + "." + name, getRevision());
      data.m_pdata = (void*)((char*)m_pdata + offset);
      ptr = data.parse(ptr->next, length);
      int len = (length == 0) ? 1 : length;
      NSMDataList data_v;
      for (int i = 0; i < len; i++) {
        NSMData cdata(data);
        cdata.setIndex(i);
        cdata.setNode(getNode());
        cdata.m_pdata = (void*)((char*)data.get() + i * data.getSize());
        data_v.push_back(cdata);
      }
      m_data_v_m.insert(NSMDataListMap::value_type(name, data_v));
      type = FieldInfo::NSM_OBJECT;
      m_size += data.m_size * length;
    } else if (type == ')') {
      return ptr;
    }
    FieldInfo::Property pro((FieldInfo::Type)type, length, offset);
    add(name, pro);
    ptr = ptr->next;
    int len = (length == 0) ? 1 : length;
    m_size += pro.getTypeSize() * len;
  }
  return 0;
}
#endif

void* NSMData::getValue(const std::string& name) throw()
{
  if (!hasValue(name)) return NULL;
  char* data = (char*)get();
  return (data + getProperty(name).getOffset());
}

const void* NSMData::getValue(const std::string& name)
const throw()
{
  if (!hasValue(name)) return NULL;
  char* data = (char*)get();
  return (data + getProperty(name).getOffset());
}

void NSMData::readObject(Reader& reader) throw(IOException)
{
  setName(reader.readString());
  setFormat(reader.readString());
  setRevision(reader.readInt());
  int size = reader.readInt();
  if (m_size > 0 && m_size < size) {
    m_allocated = true;
    free(m_pdata);
    m_pdata = malloc(size);
  }
  setSize(size);
  int npars = reader.readInt();
  for (int n = 0; n < npars; n++) {
    std::string name = reader.readString();
    FieldInfo::Type type = (FieldInfo::Type)reader.readInt();
    size_t length = reader.readInt();
    size_t offset = reader.readInt();
    if (!hasValue(name)) add(name, FieldInfo::Property(type, length, offset));
    void* buf = getValue(name);
    if (length == 0) length = 1;
    if (type == FieldInfo::NSM_OBJECT) {
      NSMDataList data_v;
      for (size_t i = 0; i < length; i++) {
        NSMData data;
        data.m_pdata = (char*)getValue(name) + length * i;
        reader.readObject(data);
        data_v.push_back(data);
      }
      m_data_v_m.insert(NSMDataListMap::value_type(name, data_v));
    } else {
      for (size_t i = 0; i < length; i++) {
        switch (type) {
          case FieldInfo::NSM_CHAR: ((char*)buf)[i] = reader.readChar(); break;
          case FieldInfo::NSM_INT16: ((int16*)buf)[i] = reader.readShort(); break;
          case FieldInfo::NSM_INT32: ((int32*)buf)[i] = reader.readInt(); break;
          case FieldInfo::NSM_INT64: ((int64*)buf)[i] = reader.readLong(); break;
          case FieldInfo::NSM_BYTE8: ((byte8*)buf)[i] = reader.readChar(); break;
          case FieldInfo::NSM_UINT16: ((uint16*)buf)[i] = reader.readShort(); break;
          case FieldInfo::NSM_UINT32: ((uint32*)buf)[i] = reader.readInt(); break;
          case FieldInfo::NSM_UINT64: ((uint64*)buf)[i] = reader.readLong(); break;
          case FieldInfo::NSM_FLOAT: ((float*)buf)[i] = reader.readFloat(); break;
          case FieldInfo::NSM_DOUBLE: ((double*)buf)[i] = reader.readDouble(); break;
          default: break;
        }
      }
    }
  }
}

void NSMData::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeString(getName());
  writer.writeString(getFormat());
  writer.writeInt(getRevision());
  writer.writeInt(getSize());
  const FieldNameList& name_v(getFieldNames());
  writer.writeInt(name_v.size());
  for (FieldNameList::const_iterator it = name_v.begin();
       it != name_v.end(); it++) {
    const std::string& name(*it);
    writer.writeString(name);
    const FieldInfo::Property& pro(getProperty(name));
    writer.writeInt(pro.getType());
    writer.writeInt(pro.getLength());
    writer.writeInt(pro.getOffset());
    size_t length = pro.getLength();
    const void* buf = getValue(name);
    if (length == 0) length = 1;
    if (pro.getType() == FieldInfo::NSM_OBJECT) {
      const NSMDataList& data_v(getObjects(name));
      for (size_t i = 0; i < length; i++) {
        writer.writeObject(data_v[i]);
      }
    } else {
      for (size_t i = 0; i < length; i++) {
        switch (pro.getType()) {
          case FieldInfo::NSM_CHAR:   writer.writeChar(((char*)buf)[i]); break;
          case FieldInfo::NSM_INT16:  writer.writeShort(((int16*)buf)[i]); break;
          case FieldInfo::NSM_INT64:  writer.writeLong(((int64*)buf)[i]); break;
          case FieldInfo::NSM_INT32:  writer.writeInt(((int32*)buf)[i]); break;
          case FieldInfo::NSM_BYTE8:  writer.writeChar(((byte8*)buf)[i]); break;
          case FieldInfo::NSM_UINT16: writer.writeShort(((uint16*)buf)[i]); break;
          case FieldInfo::NSM_UINT32: writer.writeInt(((uint32*)buf)[i]); break;
          case FieldInfo::NSM_UINT64: writer.writeLong(((uint64*)buf)[i]); break;
          case FieldInfo::NSM_FLOAT:  writer.writeFloat(((float*)buf)[i]); break;
          case FieldInfo::NSM_DOUBLE: writer.writeDouble(((double*)buf)[i]); break;
          default : break;
        }
      }
    }
  }
}

int NSMData::getNObjects(const std::string& name) const throw()
{
  return m_data_v_m[name].size();
}

const DBObject& NSMData::getObject(const std::string& name, int index) const throw()
{
  return m_data_v_m[name][index];
}

DBObject& NSMData::getObject(const std::string& name, int index) throw()
{
  return m_data_v_m[name][index];
}

const NSMData::NSMDataList& NSMData::getObjects(const std::string& name) const throw()
{
  return m_data_v_m[name];
}

NSMData::NSMDataList& NSMData::getObjects(const std::string& name) throw()
{
  return m_data_v_m[name];
}

