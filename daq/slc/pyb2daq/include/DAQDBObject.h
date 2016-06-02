#ifndef _Belle2_DAQDBObject_h
#define _Belle2_DAQDBObject_h

#include <daq/slc/database/DBObject.h>

#include <boost/python.hpp>

namespace Belle2 {

  class DAQDBObject {

  public:
    DAQDBObject() {}
    DAQDBObject(const DBObject& obj) : m_obj(obj) {}
    DAQDBObject(const char* tablename, const char* configname);

  public:
    void create(const char* tablename);
    boost::python::list getDBList(const char* tablename, const char* prefix);
    const std::string& getPath() const throw() { return m_obj.getPath(); }
    void setPath(const std::string& path) throw() { m_obj.setPath(path); }
    int getId() const throw() { return m_obj.getId(); }
    void setId(int id) throw() { m_obj.setId(id); }
    const std::string& getName() const throw() { return m_obj.getName(); }
    void setName(const std::string& name) throw() { m_obj.setName(name); }
    int getIndex() const { return m_obj.getIndex(); }
    void setIndex(int index) { m_obj.setIndex(index); }
    boost::python::list getFieldNames() const throw();
    boost::python::list getNameList(bool showall) const throw();
    DBField::Type getType(const std::string& name) const throw(std::out_of_range)
    {
      return m_obj.getProperty(name).getType();
    }
    void print() const throw() { m_obj.print(); }
    std::string sprint() const throw() { return m_obj.sprint(true); }
    bool hasField(const std::string& name) const throw() { return m_obj.hasField(name); }
    bool hasValue(const std::string& name) const throw() { return m_obj.hasValue(name); }
    bool hasText(const std::string& name) const throw() { return m_obj.hasText(name); }
    bool hasObject(const std::string& name) const throw() { return hasObject(name); }
    void setText(const std::string& name, const std::string& value) throw(std::out_of_range) { m_obj.setText(name, value); }
    void addText(const std::string& name, const std::string& value) throw(std::out_of_range) { m_obj.addText(name, value); }

  public:
    DAQDBObject getObject(const std::string& name) throw(std::out_of_range) { return m_obj(name); }
    DAQDBObject getObjects(const std::string& name, int index) throw(std::out_of_range) { return m_obj(name, index); }
    bool getBool(const std::string& name) const throw(std::out_of_range) { return m_obj.getBool(name); }
    char getChar(const std::string& name) const throw(std::out_of_range) { return m_obj.getChar(name); }
    short getShort(const std::string& name) const throw(std::out_of_range) { return m_obj.getShort(name); }
    int getInt(const std::string& name) const throw(std::out_of_range)  { return m_obj.getInt(name); }
    long long getLong(const std::string& name) const throw(std::out_of_range) { return m_obj.getLong(name); }
    float getFloat(const std::string& name) const throw(std::out_of_range) { return m_obj.getFloat(name); }
    double getDouble(const std::string& name) const throw(std::out_of_range) { return m_obj.getDouble(name); }
    void addBool(const std::string& name, bool value) throw() { m_obj.addBool(name, value); }
    void addChar(const std::string& name, char value) throw() { m_obj.addChar(name, value); }
    void addShort(const std::string& name, short value) throw() { m_obj.addShort(name, value); }
    void addInt(const std::string& name, int value) throw()  { m_obj.addInt(name, value); }
    void addLong(const std::string& name, long long value) throw()  { m_obj.addLong(name, value); }
    void addFloat(const std::string& name, float value) throw() { m_obj.addFloat(name, value); }
    void addDouble(const std::string& name, double value) throw() { m_obj.addDouble(name, value); }
    void setBool(const std::string& name, bool value) throw(std::out_of_range) { m_obj.setBool(name, value); }
    void setChar(const std::string& name, int value) throw(std::out_of_range) { m_obj.setChar(name, value); }
    void setShort(const std::string& name, int value) throw(std::out_of_range) { m_obj.setShort(name, value); }
    void setInt(const std::string& name, int value) throw(std::out_of_range) { m_obj.setInt(name, value); }
    void setLong(const std::string& name, long long value) throw(std::out_of_range) { m_obj.setLong(name, value); }
    void setFloat(const std::string& name, float value) throw(std::out_of_range) { m_obj.setFloat(name, value); }
    void setDouble(const std::string& name, double value) throw(std::out_of_range) { m_obj.setDouble(name, value); }
    std::string getValueText(const std::string& name) const throw(std::out_of_range) { return m_obj.getValueText(name); }
    const std::string& getText(const std::string& name) const throw(std::out_of_range) { return m_obj.getText(name); }
    int getNObjects(const std::string& name) const throw() { return m_obj.getNObjects(name); }
    void addObject(const std::string& name, const DAQDBObject& obj) throw() { m_obj.addObject(name, obj.m_obj); }

  private:
    DBObject m_obj;

  };

}

#endif
