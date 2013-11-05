#ifndef _Belle2_DataObject_hh
#define _Belle2_DataObject_hh

#include "base/Command.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class ConfigFile;

  class DataObject {

    typedef std::map<std::string, std::map<std::string, int> > EnumMap;

  public:
    enum ParamType {
      BOOLEAN, INT, ENUM, TEXT, OBJECT
    };

    enum ParamPriority {
      NONE, BOOT, LOAD, TRIGFT
    };

  public:
    DataObject();
    DataObject(DataObject* obj) {
      if (obj != NULL)
        *this = *obj;
    }
    DataObject(const std::string& data_class,
               const std::string& base_class);
    ~DataObject() throw() {}

  public:
    void setRevision(int revision) { _revision = revision; }
    int getRevision() const { return _revision; }
    const std::string& getClassName() { return _class; }
    const std::string& getBaseClassName() { return _base_class; }
    std::vector<std::string>& getParamNames() { return _param_name_v; }
    std::vector<ParamType>& getParamTypes() { return _param_type_v; }
    const std::string getParamName(int i) { return _param_name_v[i]; }
    ParamType getParamType(int i) { return _param_type_v[i]; }
    ParamPriority getParamPriority(int i) { return _param_priority_v[i]; }
    std::map<std::string, bool>& getBooleanValues() { return _bool_value_m; }
    std::map<std::string, int>& getIntValues() { return _int_value_m; }
    std::map<std::string, std::string>& getTextValues() { return _text_value_m; }
    std::map<std::string, DataObject*>& getObjects() { return _object_m; }
    bool getBooleanValue(const std::string& name) const { return _bool_value_m[name]; }
    int getIntValue(const std::string& name) const { return _int_value_m[name]; }
    int getEnumValue(const std::string& name) const { return _int_value_m[name]; }
    const std::string& getTextValue(const std::string& name) const { return _text_value_m[name]; }
    DataObject* getObject(const std::string& name) { return _object_m[name]; }
    void addBoolean(const std::string& name, bool value,
                    ParamPriority priority = LOAD) {
      _param_name_v.push_back(name);
      _param_type_v.push_back(BOOLEAN);
      _param_priority_v.push_back(priority);
      _bool_value_m.insert(std::map<std::string, bool>::value_type(name, value));
    }
    void addInt(const std::string& name, int value,
                ParamPriority priority = LOAD) {
      _param_name_v.push_back(name);
      _param_type_v.push_back(INT);
      _param_priority_v.push_back(priority);
      _int_value_m.insert(std::map<std::string, int>::value_type(name, value));
    }
    void addEnum(const std::string& name, std::map<std::string, int>& enum_m,
                 const std::string& value, ParamPriority priority = LOAD) {
      _param_name_v.push_back(name);
      _param_type_v.push_back(ENUM);
      _param_priority_v.push_back(priority);
      _enum_m_m.insert(EnumMap::value_type(name, enum_m));
      _int_value_m.insert(std::map<std::string, int>::value_type(name, enum_m[value]));
    }
    void addText(const std::string& name, const std::string& value,
                 ParamPriority priority = LOAD) {
      _param_name_v.push_back(name);
      _param_type_v.push_back(TEXT);
      _param_priority_v.push_back(priority);
      _text_value_m.insert(std::map<std::string, std::string>::value_type(name, value));
    }
    void addObject(const std::string& name, DataObject* value,
                   ParamPriority priority = LOAD) {
      _param_name_v.push_back(name);
      _param_type_v.push_back(OBJECT);
      _param_priority_v.push_back(priority);
      _object_m.insert(std::map<std::string, DataObject*>::value_type(name, value));
    }
    void setBooleanValue(const std::string& name, bool value) {
      _bool_value_m[name] = value;
    }
    void setIntValue(const std::string& name, int value) {
      _int_value_m[name] = value;
    }
    void setEnumValue(const std::string& name, const std::string& value) {
      _int_value_m[name] = _enum_m_m[name][value];
    }
    void setTextValue(const std::string& name, const std::string& value) {
      _text_value_m[name] =  value;
    }
    void setObject(const std::string& name, DataObject* value) {
      _object_m[name] =  value;
    }

  public:
    int setToMessage(ParamPriority priority, unsigned int* pars,
                     int par_i, std::vector<std::string>& data_p);
    int getFromMessage(ParamPriority priority, const unsigned int* pars,
                       int par_i, std::vector<std::string>& data_p);
    int setToMessage(const Command& command, unsigned int* pars,
                     int par_i, std::vector<std::string>& data_p);
    int getFromMessage(const Command& command, const unsigned int* pars,
                       int par_i, std::vector<std::string>& data_p);
    void print();
    const std::string toSQLConfig();
    const std::string toSQLNames();
    const std::string toSQLValues();
    void setSQLValues(std::vector<std::string>& name_v,
                      std::vector<std::string>& value_v);
    const std::string getValueString();
    void readValueString(ConfigFile& config);

  private:
    int _revision;
    std::string _class;
    std::string _base_class;
    std::vector<std::string> _param_name_v;
    std::vector<ParamType> _param_type_v;
    std::vector<ParamPriority> _param_priority_v;
    mutable std::map<std::string, bool> _bool_value_m;
    mutable std::map<std::string, int> _int_value_m;
    mutable std::map<std::string, std::string> _text_value_m;
    std::map<std::string, DataObject*> _object_m;
    EnumMap _enum_m_m;

  };

}

#endif
