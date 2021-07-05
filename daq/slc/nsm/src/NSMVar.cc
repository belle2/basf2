/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMVar.h"

#include <string.h>
#include <stdlib.h>

#include <daq/slc/base/Reader.h>
#include <daq/slc/base/Writer.h>

using namespace Belle2;

NSMVar::NSMVar(const std::string& name, const std::vector<int>& value)
  : m_value(NULL)
{
  int* v = (int*)malloc(value.size() * sizeof(int));
  for (size_t i = 0; i < value.size(); i++)
    v[i] = value[i];
  m_name = name;
  m_type = INT;
  m_len = value.size();
  m_value = v;
  m_id = 0;
  m_date = Date().get();
}

NSMVar::NSMVar(const std::string& name, const std::vector<float>& value)
  : m_value(NULL)
{
  float* v = (float*)malloc(value.size() * sizeof(float));
  for (size_t i = 0; i < value.size(); i++)
    v[i] = value[i];
  m_name = name;
  m_type = INT;
  m_len = value.size();
  m_value = v;
  m_id = 0;
  m_date = Date().get();
}

const NSMVar& NSMVar::operator=(const std::vector<int>& val)
{
  if (m_value) free(m_value);
  int* v = (int*)malloc(val.size() * sizeof(int));
  for (size_t i = 0; i < val.size(); i++)
    v[i] = val[i];
  m_value = v;
  m_type = INT;
  m_len = val.size();
  m_date = Date().get();
  return *this;
}

const NSMVar& NSMVar::operator=(const std::vector<float>& val)
{
  if (m_value) free(m_value);
  float* v = (float*)malloc(val.size() * sizeof(int));
  for (size_t i = 0; i < val.size(); i++)
    v[i] = val[i];
  m_value = v;
  m_type = FLOAT;
  m_len = val.size();
  m_date = Date().get();
  return *this;
}

int NSMVar::size() const
{
  int s = 1;
  switch (m_type) {
    case NONE: s = 0; return 0;
    case INT: s = 4; break;
    case FLOAT: s = 4; break;
    case TEXT: return m_len + 1;
    default:  break;
  }
  return (m_len > 0) ? s * m_len : s;
}

const char* NSMVar::getTypeLabel() const
{
  switch (m_type) {
    case INT:
      if (m_len > 0) return "inta";
      else return "int";
    case FLOAT:
      if (m_len > 0) return "floata";
      else return "float";
    case TEXT: return "text";
    default:  break;
  }
  return "";
}

void NSMVar::copy(const std::string& name,
                  Type type, int len, const void* value,
                  int id, int date)
{
  if (m_value) free(m_value);
  m_value = NULL;
  m_name = name;
  m_type = type;
  m_len = len;
  m_id = id;
  m_date = (date > 0) ? date : Date().get();
  int s = size();
  if (s > 0) {
    m_value = malloc(s);
    memcpy(m_value, value, s);
  } else {
    m_type = NONE;
    m_len = 0;
    m_value = NULL;
  }
}

NSMVar::~NSMVar()
{
  if (m_value) free(m_value);
}

void NSMVar::readObject(Reader& reader)
{
  m_node = reader.readString();
  m_name = reader.readString();
  m_type = (Type)reader.readInt();
  m_len = reader.readInt();
  m_id = reader.readInt();
  m_date = reader.readInt();
  int len = (m_len > 0) ? m_len : 1;
  int s = size();
  if (s > 0) {
    if (m_value) free(m_value);
    m_value = malloc(s);
    memset(m_value, 0, s);
    switch (m_type) {
      case INT: {
        int* v = (int*)m_value;
        for (int i = 0; i < len; i++) {
          v[i] = reader.readInt();
        }
      } break;
      case FLOAT: {
        float* v = (float*)m_value;
        for (int i = 0; i < len; i++) {
          v[i] = reader.readFloat();
        }
      } break;
      case TEXT: {
        char* v = (char*)m_value;
        // Use m_len condition instead of len due to empty string problem
        for (int i = 0; i < m_len; i++) {
          v[i] = reader.readChar();
        }
      } break;
      default: return;
    }
  }
}

void NSMVar::writeObject(Writer& writer) const
{
  writer.writeString(m_node);
  writer.writeString(m_name);
  writer.writeInt((int)m_type);
  writer.writeInt(m_len);
  writer.writeInt(m_id);
  writer.writeInt(m_date);
  int len = (m_len > 0) ? m_len : 1;
  switch (m_type) {
    case INT: {
      int* v = (int*)m_value;
      for (int i = 0; i < len; i++) {
        writer.writeInt(v[i]);
      }
    } break;
    case FLOAT: {
      float* v = (float*)m_value;
      for (int i = 0; i < len; i++) {
        writer.writeFloat(v[i]);
      }
    } break;
    case TEXT: {
      char* v = (char*)m_value;
      writer.write(v, len);
    } break;
    default: return;
  }
}

int NSMVar::getInt() const
{
  return (m_type == INT&& m_len == 0) ? *(int*)m_value : 0;
}

float NSMVar::getFloat() const
{
  return (m_type == FLOAT && m_len == 0) ? *(float*)m_value : 0;
}

const char* NSMVar::getText() const
{
  return (m_type == TEXT && m_len > 0 && m_value != NULL) ? (const char*)m_value : "";
}

int NSMVar::getInt(int i) const
{
  return (m_type == INT&& i < m_len) ? ((int*)m_value)[i] : 0;
}

float NSMVar::getFloat(int i) const
{
  return (m_type == FLOAT && i < m_len) ? ((float*)m_value)[i] : 0;
}

const NSMVar& NSMVar::operator>>(std::vector<int>& val) const
{
  int* pv = (int*)get();
  val = std::vector<int>();
  for (int i = 0; i < getLength(); i++) {
    val.push_back(pv[i]);
  }
  return *this;
}

const NSMVar& NSMVar::operator>>(std::vector<float>& val) const
{
  float* pv = (float*)get();
  val = std::vector<float>();
  for (int i = 0; i < getLength(); i++) {
    val.push_back(pv[i]);
  }
  return *this;
}
