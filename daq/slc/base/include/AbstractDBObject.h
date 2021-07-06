/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_AbstractDBObject_hh
#define _Belle2_AbstractDBObject_hh

#include <daq/slc/base/FieldProperty.h>

#include <daq/slc/base/Serializable.h>

#include <string>
#include <stdexcept>

namespace Belle2 {


  class AbstractDBObject : public Serializable {

  public:
    AbstractDBObject();
    AbstractDBObject(const AbstractDBObject& obj);
    virtual ~AbstractDBObject();

  public:
    const std::string& getPath() const { return m_path; }
    void setPath(const std::string& path) { m_path = path; }
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }
    DBField::NameList& getFieldNames() { return m_name_v; }
    const DBField::NameList& getFieldNames() const { return m_name_v; }
    DBField::Property getProperty(const std::string& name) const;
    bool hasField(const std::string& name) const;
    bool hasValue(const std::string& name) const;
    bool hasText(const std::string& name) const;
    bool hasObject(const std::string& name) const;
    void add(const std::string& name, DBField::Property pro);
    void setValue(const std::string& name, const std::string& value);
    void setText(const std::string& name, const std::string& value) { addText(name, value); }

  public:
    bool getBool(const std::string& name, int index = 0) const;
    char getChar(const std::string& name, int index = 0) const;
    short getShort(const std::string& name, int index = 0) const;
    int getInt(const std::string& name, int index = 0) const;
    long long getLong(const std::string& name, int index = 0) const;
    float getFloat(const std::string& name, int index = 0) const;
    double getDouble(const std::string& name, int index = 0) const;
    void addBool(const std::string& name, bool value);
    void addChar(const std::string& name, char value);
    void addShort(const std::string& name, short value);
    void addInt(const std::string& name, int value);
    void addLong(const std::string& name, long long value);
    void addFloat(const std::string& name, float value);
    void addDouble(const std::string& name, double value);
    void setBool(const std::string& name, bool value, int index = 0);
    void setChar(const std::string& name, int value, int index = 0);
    void setShort(const std::string& name, int value, int index = 0);
    void setInt(const std::string& name, int value, int index = 0);
    void setLong(const std::string& name, long long value, int index = 0);
    void setFloat(const std::string& name, float value, int index = 0);
    void setDouble(const std::string& name, double value, int index = 0);
    const std::string getValueText(const std::string& name) const;

  public:
    virtual const void* getValue(const std::string& name) const = 0;
    virtual const std::string& getText(const std::string& name) const = 0;
    virtual void addText(const std::string& name, const std::string& value) = 0;
    virtual void addValue(const std::string& name, const void* value,
                          DBField::Type type, int length) = 0;
    virtual void setValue(const std::string& name, const void* value, int index) = 0;
    virtual void setValueText(const std::string& name, const std::string& value);

  protected:
    virtual void reset();

  private:
    int m_index;
    std::string m_path;
    int m_id;
    std::string m_name;
    DBField::NameList m_name_v;
    DBField::PropertyList m_pro_m;

  private:
    template<typename T>
    T getD(const std::string& name, int index = 0) const
    {
      const void* value = getValue(name);
      if (value == NULL/* || index >= getProperty(name).getLength()*/) {
        throw (std::out_of_range(name + " not found"));
      }
      DBField::Type type(getProperty(name).getType());
      if (type == DBField::INT) return (T)(((int*)value)[index]);
      if (type == DBField::FLOAT) return (T)(((float*)value)[index]);
      if (type == DBField::DOUBLE) return (T)(((double*)value)[index]);
      if (type == DBField::SHORT) return (T)(((short*)value)[index]);
      if (type == DBField::CHAR) return (T)(((char*)value)[index]);
      if (type == DBField::LONG) return (T)(((long*)value)[index]);
      return ((T*)value)[index];
    }

  };

  inline bool AbstractDBObject::getBool(const std::string& name, int index) const
  {
    return getD<bool>(name, index);
  }

  inline char AbstractDBObject::getChar(const std::string& name, int index) const
  {
    return getD<char>(name, index);
  }

  inline short AbstractDBObject::getShort(const std::string& name, int index) const
  {
    return getD<short>(name, index);
  }

  inline int AbstractDBObject::getInt(const std::string& name, int index) const
  {
    return getD<int>(name, index);
  }

  inline long long AbstractDBObject::getLong(const std::string& name, int index) const
  {
    return getD<long long>(name, index);
  }

  inline float AbstractDBObject::getFloat(const std::string& name, int index) const
  {
    return getD<float>(name, index);
  }

  inline double AbstractDBObject::getDouble(const std::string& name, int index) const
  {
    return getD<double>(name, index);
  }

  inline void AbstractDBObject::setBool(const std::string& name, bool value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setChar(const std::string& name, int value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setShort(const std::string& name, int value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setInt(const std::string& name, int value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setLong(const std::string& name, long long value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setFloat(const std::string& name, float value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setDouble(const std::string& name, double value, int index)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::addBool(const std::string& name, bool value)
  {
    addValue(name, &value, DBField::BOOL, 0);
  }

  inline void AbstractDBObject::addChar(const std::string& name, char value)
  {
    addValue(name, &value, DBField::CHAR, 0);
  }

  inline void AbstractDBObject::addShort(const std::string& name, short value)
  {
    addValue(name, &value, DBField::SHORT, 0);
  }

  inline void AbstractDBObject::addInt(const std::string& name, int value)
  {
    addValue(name, &value, DBField::INT, 0);
  }

  inline void AbstractDBObject::addLong(const std::string& name, long long value)
  {
    addValue(name, &value, DBField::LONG, 0);
  }

  inline void AbstractDBObject::addFloat(const std::string& name, float value)
  {
    addValue(name, &value, DBField::FLOAT, 0);
  }

  inline void AbstractDBObject::addDouble(const std::string& name, double value)
  {
    addValue(name, &value, DBField::DOUBLE, 0);
  }

}

#endif
