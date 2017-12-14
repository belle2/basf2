#ifndef _Belle2_AbstractDBObject_hh
#define _Belle2_AbstractDBObject_hh

#include <daq/slc/base/FieldProperty.h>

#include <daq/slc/base/Serializable.h>

#include <string>
#include <vector>
#include <map>
#include <stdexcept>

namespace Belle2 {


  class AbstractDBObject : public Serializable {

  public:
    AbstractDBObject();
    AbstractDBObject(const AbstractDBObject& obj);
    virtual ~AbstractDBObject() throw();

  public:
    const std::string& getPath() const throw() { return m_path; }
    void setPath(const std::string& path) throw() { m_path = path; }
    int getId() const throw() { return m_id; }
    void setId(int id) throw() { m_id = id; }
    const std::string& getName() const throw() { return m_name; }
    void setName(const std::string& name) throw() { m_name = name; }
    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }
    DBField::NameList& getFieldNames() throw() { return m_name_v; }
    const DBField::NameList& getFieldNames() const throw() { return m_name_v; }
    DBField::Property getProperty(const std::string& name) const throw();
    bool hasField(const std::string& name) const throw();
    bool hasValue(const std::string& name) const throw();
    bool hasText(const std::string& name) const throw();
    bool hasObject(const std::string& name) const throw();
    void add(const std::string& name, DBField::Property pro) throw();
    void setValue(const std::string& name, const std::string& value) throw(std::out_of_range);
    void setText(const std::string& name, const std::string& value) throw(std::out_of_range) { addText(name, value); }

  public:
    bool getBool(const std::string& name, int index = 0) const throw(std::out_of_range);
    char getChar(const std::string& name, int index = 0) const throw(std::out_of_range);
    short getShort(const std::string& name, int index = 0) const throw(std::out_of_range);
    int getInt(const std::string& name, int index = 0) const throw(std::out_of_range);
    long long getLong(const std::string& name, int index = 0) const throw(std::out_of_range);
    float getFloat(const std::string& name, int index = 0) const throw(std::out_of_range);
    double getDouble(const std::string& name, int index = 0) const throw(std::out_of_range);
    void addBool(const std::string& name, bool value) throw();
    void addChar(const std::string& name, char value) throw();
    void addShort(const std::string& name, short value) throw();
    void addInt(const std::string& name, int value) throw();
    void addLong(const std::string& name, long long value) throw();
    void addFloat(const std::string& name, float value) throw();
    void addDouble(const std::string& name, double value) throw();
    void setBool(const std::string& name, bool value, int index = 0) throw(std::out_of_range);
    void setChar(const std::string& name, int value, int index = 0) throw(std::out_of_range);
    void setShort(const std::string& name, int value, int index = 0) throw(std::out_of_range);
    void setInt(const std::string& name, int value, int index = 0) throw(std::out_of_range);
    void setLong(const std::string& name, long long value, int index = 0) throw(std::out_of_range);
    void setFloat(const std::string& name, float value, int index = 0) throw(std::out_of_range);
    void setDouble(const std::string& name, double value, int index = 0) throw(std::out_of_range);
    const std::string getValueText(const std::string& name) const throw(std::out_of_range);

  public:
    virtual const void* getValue(const std::string& name) const throw(std::out_of_range) = 0;
    virtual const std::string& getText(const std::string& name) const throw(std::out_of_range) = 0;
    virtual void addText(const std::string& name, const std::string& value) throw() = 0;
    virtual void addValue(const std::string& name, const void* value,
                          DBField::Type type, int length) throw() = 0;
    virtual void setValue(const std::string& name, const void* value, int index) throw() = 0;
    virtual void setValueText(const std::string& name, const std::string& value) throw(std::out_of_range);

  protected:
    virtual void reset() throw();

  private:
    int m_index;
    std::string m_path;
    int m_id;
    std::string m_name;
    DBField::NameList m_name_v;
    DBField::PropertyList m_pro_m;

  private:
    template<typename T>
    T getD(const std::string& name, int index = 0) const throw(std::out_of_range)
    {
      const void* value = getValue(name);
      if (value == NULL/* || index >= getProperty(name).getLength()*/) {
        throw (std::out_of_range(name + " not found"));
      }
      return ((T*)value)[index];
    }

  };

  inline bool AbstractDBObject::getBool(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<bool>(name, index);
  }

  inline char AbstractDBObject::getChar(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<char>(name, index);
  }

  inline short AbstractDBObject::getShort(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<short>(name, index);
  }

  inline int AbstractDBObject::getInt(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<int>(name, index);
  }

  inline long long AbstractDBObject::getLong(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<long long>(name, index);
  }

  inline float AbstractDBObject::getFloat(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<float>(name, index);
  }

  inline double AbstractDBObject::getDouble(const std::string& name, int index) const throw(std::out_of_range)
  {
    return getD<double>(name, index);
  }

  inline void AbstractDBObject::setBool(const std::string& name, bool value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setChar(const std::string& name, int value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setShort(const std::string& name, int value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setInt(const std::string& name, int value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setLong(const std::string& name, long long value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setFloat(const std::string& name, float value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::setDouble(const std::string& name, double value, int index) throw(std::out_of_range)
  {
    setValue(name, &value, index);
  }

  inline void AbstractDBObject::addBool(const std::string& name, bool value) throw()
  {
    addValue(name, &value, DBField::BOOL, 0);
  }

  inline void AbstractDBObject::addChar(const std::string& name, char value) throw()
  {
    addValue(name, &value, DBField::CHAR, 0);
  }

  inline void AbstractDBObject::addShort(const std::string& name, short value) throw()
  {
    addValue(name, &value, DBField::SHORT, 0);
  }

  inline void AbstractDBObject::addInt(const std::string& name, int value) throw()
  {
    addValue(name, &value, DBField::INT, 0);
  }

  inline void AbstractDBObject::addLong(const std::string& name, long long value) throw()
  {
    addValue(name, &value, DBField::LONG, 0);
  }

  inline void AbstractDBObject::addFloat(const std::string& name, float value) throw()
  {
    addValue(name, &value, DBField::FLOAT, 0);
  }

  inline void AbstractDBObject::addDouble(const std::string& name, double value) throw()
  {
    addValue(name, &value, DBField::DOUBLE, 0);
  }

}

#endif
