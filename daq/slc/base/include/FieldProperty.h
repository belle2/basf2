/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_FieldProperty_hh
#define _Belle2_FieldProperty_hh

#include <string>
#include <map>
#include <vector>

namespace Belle2 {

  namespace DBField {

    enum Type {
      UNKNOWN = 0,
      BOOL = 1,
      CHAR,
      SHORT,
      INT,
      LONG,
      FLOAT,
      DOUBLE,
      TEXT,
      OBJECT
    };

    class Property {

    public:
      Property():  m_type(), m_length(0), m_offset(0) {}
      Property(Type type, int length, int offset = 0)
        : m_type(type), m_length(length), m_offset(offset) {}
      Property(const Property& pro)
        : m_type(pro.m_type), m_length(pro.m_length), m_offset(pro.m_offset) {}
      ~Property() {}

    public:
      Type getType() const { return m_type; }
      int getLength() const { return m_length; }
      int getOffset() const { return m_offset; }
      void setType(Type type) { m_type = type; }
      void setLength(int length) { m_length = length; }

      int getTypeSize() const
      {
        switch (m_type) {
          case BOOL:   return sizeof(bool);
          case CHAR:   return sizeof(char);
          case SHORT:  return sizeof(short);
          case INT:    return sizeof(int);
          case LONG:   return sizeof(long long);
          case FLOAT:  return sizeof(float);
          case DOUBLE: return sizeof(double);
          default: break;
        }
        return 0;
      }

      const std::string getTypeText() const
      {
        switch (m_type) {
          case BOOL: return "bool";
          case CHAR: return "char";
          case SHORT: return "short";
          case INT: return "int";
          case LONG: return "long";
          case FLOAT: return "float";
          case DOUBLE: return "double";
          case TEXT: return "text";
          case OBJECT: return "object";
          default: break;
        }
        return "";
      }

    private:
      Type m_type;
      int m_length;
      int m_offset;

    };

    typedef std::vector<std::string> NameList;
    typedef std::map<std::string, DBField::Property> PropertyList;

  }

}

#endif
