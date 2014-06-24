#ifndef _Belle2_DBObject_hh
#define _Belle2_DBObject_hh

#include <daq/slc/database/FieldInfo.h>

#include <daq/slc/base/Serializable.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  typedef std::vector<std::string> FieldNameList;
  typedef std::map<std::string, FieldInfo::Property> FieldPropertyList;
  typedef std::map<std::string, int> EnumList;
  typedef std::map<std::string, EnumList> EnumNameList;

  class DBObject : public Serializable {

  public:
    DBObject();
    DBObject(const DBObject& obj);
    virtual ~DBObject() throw();

  public:
    int getId() const throw() { return m_id; }
    void setId(int id) throw() { m_id = id; }
    const std::string& getName() const throw() { return m_name; }
    void setName(const std::string& name) throw() { m_name = name; }
    const std::string& getNode() const throw() { return m_node; }
    void setNode(const std::string& node) throw() { m_node = node; }
    const std::string& getTable() const throw() { return m_table; }
    void setTable(const std::string& table) throw() { m_table = table; }
    int getIndex() const { return m_index; }
    void setIndex(int index) { m_index = index; }
    int getRevision() const throw() { return m_revision; }
    void setRevision(int revision) throw() { m_revision = revision; }
    FieldNameList& getFieldNames() throw() { return m_name_v; }
    const FieldNameList& getFieldNames() const throw() { return m_name_v; }
    FieldInfo::Property getProperty(const std::string& name) const throw();
    size_t getLength(const std::string& name) const throw();
    bool hasField(const std::string& name) const throw();
    bool hasValue(const std::string& name) const throw();
    bool hasText(const std::string& name) const throw();
    bool hasEnum(const std::string& name) const throw();
    bool hasObject(const std::string& name, int index = -1) const throw();
    bool hasArray(const std::string& name) const throw();
    int getEnumId(const std::string& name) const throw();
    const EnumList& getEnumList(const std::string& name) const throw();
    void add(const std::string& name, FieldInfo::Property pro) throw();
    void addEnumList(const std::string& name, const EnumList& enum_m) throw();
    void addEnumList(const std::string& name, const std::string& str) throw();
    void setValue(const std::string& name, const std::string& value) throw();
    void setText(const std::string& name, const std::string& value) throw() { addText(name, value); }
    void setEnum(const std::string& name, const std::string& value) throw() { addEnum(name, value); }
    void setEnum(const std::string& name, int value) throw();
    bool isConfig() const throw() { return m_isconfig; }

  public:
    void setConfig(bool isconfig) throw() { m_isconfig = isconfig; }

  public:
    virtual bool getBool(const std::string& name, int index = 0) const throw();
    virtual char getChar(const std::string& name, int index = 0) const throw();
    virtual short getShort(const std::string& name, int index = 0) const throw();
    virtual int getInt(const std::string& name, int index = 0) const throw();
    virtual long long getLong(const std::string& name, int index = 0) const throw();
    virtual unsigned char getUChar(const std::string& name, int index = 0) const throw();
    virtual unsigned short getUShort(const std::string& name, int index = 0) const throw();
    virtual unsigned int getUInt(const std::string& name, int index = 0) const throw();
    virtual unsigned long long getULong(const std::string& name, int index = 0) const throw();
    virtual float getFloat(const std::string& name, int index = 0) const throw();
    virtual double getDouble(const std::string& name, int index = 0) const throw();
    virtual void addBool(const std::string& name, bool value) throw() { addValue(name, &value, FieldInfo::BOOL, 0); }
    virtual void addChar(const std::string& name, char value) throw() { addValue(name, &value, FieldInfo::CHAR, 0); }
    virtual void addShort(const std::string& name, short value) throw() { addValue(name, &value, FieldInfo::SHORT, 0); }
    virtual void addInt(const std::string& name, int value) throw() { addValue(name, &value, FieldInfo::INT, 0); }
    virtual void addLong(const std::string& name, long long value) throw() { addValue(name, &value, FieldInfo::LONG, 0); }
    virtual void addUChar(const std::string& name, char value) throw() { addValue(name, &value, FieldInfo::CHAR, 0); }
    virtual void addUShort(const std::string& name, short value) throw() { addValue(name, &value, FieldInfo::SHORT, 0); }
    virtual void addUInt(const std::string& name, int value) throw() { addValue(name, &value, FieldInfo::INT, 0); }
    virtual void addULong(const std::string& name, long long value) throw() { addValue(name, &value, FieldInfo::LONG, 0); }
    virtual void addFloat(const std::string& name, float value) throw() { addValue(name, &value, FieldInfo::FLOAT, 0); }
    virtual void addDouble(const std::string& name, double value) throw() { addValue(name, &value, FieldInfo::DOUBLE, 0); }
    virtual void setBool(const std::string& name, bool value, int index = 0) throw();
    virtual void setChar(const std::string& name, int value, int index = 0) throw();
    virtual void setShort(const std::string& name, int value, int index = 0) throw();
    virtual void setInt(const std::string& name, int value, int index = 0) throw();
    virtual void setLong(const std::string& name, long long value, int index = 0) throw();
    virtual void setUChar(const std::string& name, unsigned int value, int index = 0) throw();
    virtual void setUShort(const std::string& name, unsigned int value, int index = 0) throw();
    virtual void setUInt(const std::string& name, unsigned int value, int index = 0) throw();
    virtual void setULong(const std::string& name, unsigned long long value, int index = 0) throw();
    virtual void setFloat(const std::string& name, float value, int index = 0) throw();
    virtual void setDouble(const std::string& name, double value, int index = 0) throw();

  public:
    void print() const throw();
    virtual void reset() throw();
    const std::string getIdTitle() const throw() { return (isConfig() ? "configid" : "loggerid"); }
    virtual const void* getValue(const std::string& name) const throw() = 0;
    virtual const std::string getText(const std::string& name) const throw() = 0;
    virtual void addText(const std::string& name, const std::string& value) throw() = 0;
    virtual void addEnum(const std::string& name, const std::string& value) throw() = 0;
    virtual void addValue(const std::string& name, const void* value,
                          FieldInfo::Type type, int length) throw() = 0;
    virtual void setValue(const std::string& name, const void* value, int index) throw() = 0;
    virtual void setValueText(const std::string& name, const std::string& value) throw();
    virtual int getNObjects(const std::string& name) const throw() = 0;
    virtual DBObject& getObject(const std::string& name, int i = 0) throw() = 0;
    virtual const DBObject& getObject(const std::string& name, int i = 0) const throw() = 0;
    virtual void setObject(const std::string& name, int index, const DBObject& obj) throw() = 0;
    virtual const std::string getEnum(const std::string& name) const throw() = 0;
    virtual const std::string getValueText(const std::string& name) const throw();

  private:
    int m_index;
    int m_id;
    int m_revision;
    std::string m_name;
    std::string m_node;
    std::string m_table;
    bool m_isconfig;
    mutable FieldNameList m_name_v;
    mutable FieldPropertyList m_pro_m;
    mutable EnumNameList m_enum_m_m;

  };

}

#endif
