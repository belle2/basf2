#ifndef _Belle2_ConfigObject_hh
#define _Belle2_ConfigObject_hh

#include <daq/slc/database/DBObject.h>

namespace Belle2 {

  class ConfigObject : public DBObject {

  public:
    typedef std::vector<ConfigObject> ConfigObjectList;
    typedef std::map<std::string, void*> FieldValueList;
    typedef std::map<std::string, std::string> FieldTextList;
    typedef std::map<std::string, ConfigObjectList> FieldObjectList;

  public:
    ConfigObject();
    ConfigObject(const ConfigObject& obj);
    virtual ~ConfigObject() throw();
    const ConfigObject& operator=(const ConfigObject& obj) throw();
    void copy(const ConfigObject& obj);

  public:
    ConfigObjectList& getObjects(const std::string& name) throw();
    const ConfigObjectList& getObjects(const std::string& name) const throw();
    void addObjects(const std::string& name, const ConfigObjectList& obj) throw();
    void addEnum(const std::string& name, const std::string& value,
                 const EnumList& enum_m) throw();
    void addEnum(const std::string& name, const std::string& value) throw();

  public:
    virtual void reset() throw();
    virtual const void* getValue(const std::string& name) const throw();
    virtual const std::string getText(const std::string& name) const throw();
    virtual const std::string getEnum(const std::string& name) const throw();
    virtual void addText(const std::string& name, const std::string& value) throw();
    virtual void addValue(const std::string& name, const void* value,
                          FieldInfo::Type type, int length) throw();
    virtual void setValue(const std::string& name, const void* value, int length) throw();
    virtual void addObject(const std::string& name, const ConfigObject& obj) throw();
    virtual int getNObjects(const std::string& name) const throw();
    virtual DBObject& getObject(const std::string& name, int i = 0) throw();
    virtual const DBObject& getObject(const std::string& name, int i = 0) const throw();
    virtual void setObject(const std::string& name, int index,
                           const DBObject& obj) throw();

  public:
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  private:
    mutable FieldValueList m_value_m;
    mutable FieldTextList m_text_m;
    mutable FieldObjectList m_obj_v_m;

  };

  typedef std::vector<ConfigObject> ConfigObjectList;

  inline int ConfigObject::getNObjects(const std::string& name) const throw()
  {
    return m_obj_v_m[name].size();
  }

  inline ConfigObjectList& ConfigObject::getObjects(const std::string& name) throw()
  {
    return m_obj_v_m[name];
  }

  inline const ConfigObject::ConfigObjectList& ConfigObject::getObjects(const std::string& name) const throw()
  {
    return m_obj_v_m[name];
  }

  inline DBObject& ConfigObject::getObject(const std::string& name, int i) throw()
  {
    return m_obj_v_m[name][i];
  }

  inline const DBObject& ConfigObject::getObject(const std::string& name, int i) const throw()
  {
    return m_obj_v_m[name][i];
  }

}

#endif
