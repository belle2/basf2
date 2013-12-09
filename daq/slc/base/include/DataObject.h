#ifndef _Belle2_DataObject_hh
#define _Belle2_DataObject_hh

#include "daq/slc/base/Command.h"
#include "daq/slc/base/IOException.h"
#include "daq/slc/base/Serializable.h"

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class ConfigFile;

  class DataObject : public Serializable {

  public:
    enum ParamType {
      BOOL, CHAR, SHORT, INT, LONG,
      UCHAR, USHORT, UINT, ULONG,
      FLOAT, DOUBLE, TEXT, ENUM, OBJECT
    };

    struct ParamInfo {
      ParamType type;
      size_t length;
      void* buf;
    };

    typedef std::vector<std::string> ParamNameList;
    typedef std::map<std::string, ParamInfo> ParamInfoMap;
    typedef std::map<std::string, int> EnumMap;
    typedef std::map<std::string, EnumMap > EnumMapMap;

  public:
    DataObject();
    DataObject(DataObject* obj);
    DataObject(const std::string& data_class,
               const std::string& base_class);
    virtual ~DataObject() throw();

  public:
    void print();
    const std::string toSQLConfig();
    const std::string toSQLNames();
    const std::string toSQLValues();
    void setValues(std::vector<std::string>& name_v, std::vector<std::string>& value_v);
    void setValue(const std::string& name, const std::string& value);
    void setValue(const std::string& name, const std::string& value, int length);
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  public:
    int getRevision() const { return _revision; }
    void setRevision(int revision) { _revision = revision; }
    int getConfigNumber() const { return _confno; }
    void setConfigNumber(int confno) { _confno = confno; }
    int getId() const { return _id; }
    void setId(int id) { _id = id; }
    //const std::string& getName() const { return _name; }
    //void setName(const std::string& name) { _name = name; }
    const std::string& getClassName() { return _class; }
    const std::string& getBaseClassName() { return _base_class; }
    ParamInfoMap& getParams() { return _param_m; }
    ParamNameList& getParamNames() { return _name_v; }

    const void* get(const std::string& name) const { return _param_m[name].buf; }
    bool getBool(const std::string& name) const { return *(const bool*)get(name); }
    char getChar(const std::string& name) const { return *(const char*)get(name); }
    short getShort(const std::string& name) const { return *(const short*)get(name); }
    int getInt(const std::string& name) const { return *(const int*)get(name); }
    int getEnum(const std::string& name) const { return *(const int*)get(name); }
    const std::string getEnumText(const std::string& name) const;
    long long getLong(const std::string& name) const { return *(const long long*)get(name); }
    unsigned char getUChar(const std::string& name) const { return *(const unsigned char*)get(name); }
    unsigned short getUShort(const std::string& name) const { return *(const unsigned short*)get(name); }
    unsigned int getUInt(const std::string& name) const { return *(const unsigned int*)get(name); }
    unsigned long long getULong(const std::string& name) const { return *(const unsigned long long*)get(name); }
    float getFloat(const std::string& name) const { return *(const float*)get(name); }
    double getDouble(const std::string& name) const { return *(const double*)get(name); }
    const std::string getText(const std::string& name) const { return (const char*)get(name); }
    DataObject* getObject(const std::string& name) { return (DataObject*)_param_m[name].buf; }
    void* getArray(const std::string& name, size_t& length) {
      length = _param_m[name].length;
      return _param_m[name].buf;
    }
    bool* getBoolArray(const std::string& name, size_t& length) { return (bool*)getArray(name, length); }
    char* getCharArray(const std::string& name, size_t& length) { return (char*)getArray(name, length); }
    short* getShortArray(const std::string& name, size_t& length) { return (short*)getArray(name, length); }
    int* getIntArray(const std::string& name, size_t& length) { return (int*)getArray(name, length); }
    int* getEnumArray(const std::string& name, size_t& length) { return (int*)getArray(name, length); }
    long long* getLongArray(const std::string& name, size_t& length) { return (long long*)getArray(name, length); }
    unsigned char* getUCharArray(const std::string& name, size_t& length) { return (unsigned char*)getArray(name, length); }
    unsigned short* getUShortArray(const std::string& name, size_t& length) { return (unsigned short*)getArray(name, length); }
    unsigned int* getUIntArray(const std::string& name, size_t& length) { return (unsigned int*)getArray(name, length); }
    unsigned long long* getULongArray(const std::string& name, size_t& length) { return (unsigned long long*)getArray(name, length); }
    float* getFloatArray(const std::string& name, size_t& length) { return (float*)getArray(name, length); }
    double* getDoubleArray(const std::string& name, size_t& length) { return (double*)getArray(name, length); }

    void add(const std::string& name, void* value, ParamType type, size_t length);
    void addBool(const std::string& name, bool value) { add(name, &value, BOOL, 0); }
    void addLong(const std::string& name, long long value) { add(name, &value, LONG, 0); }
    void addInt(const std::string& name, int value) { add(name, &value, INT, 0); }
    void addShort(const std::string& name, int value) { add(name, &value, SHORT, 0); }
    void addChar(const std::string& name, int value) { add(name, &value, CHAR, 0); }
    void addULong(const std::string& name, unsigned long long value) { add(name, &value, ULONG, 0); }
    void addUInt(const std::string& name, unsigned int value) { add(name, &value, UINT, 0); }
    void addUShort(const std::string& name, unsigned int value) { add(name, &value, USHORT, 0); }
    void addUChar(const std::string& name, unsigned char value) { add(name, &value, UCHAR, 0); }
    void addDouble(const std::string& name, double value) { add(name, &value, DOUBLE, 0); }
    void addFloat(const std::string& name, double value) { add(name, &value, FLOAT, 0); }
    void addEnum(const std::string& name, EnumMap& enum_m, const std::string& value);
    void addText(const std::string& name, const std::string& value, size_t length);
    void addText(const std::string& name, const char* value, size_t length);
    void addObject(const std::string& name, DataObject* value);
    void addBoolArray(const std::string& name, bool value, size_t length) { add(name, &value, BOOL, length); }
    void addIntArray(const std::string& name, int value, size_t length) { add(name, &value, INT, length); }
    void addShortArray(const std::string& name, int value, size_t length) { add(name, &value, SHORT, length); }
    void addCharArray(const std::string& name, int value, size_t length) { add(name, &value, CHAR, length); }
    void addUIntArray(const std::string& name, unsigned int value, size_t length) { add(name, &value, UINT, length); }
    void addUShortArray(const std::string& name, unsigned int value, size_t length) { add(name, &value, USHORT, length); }
    void addUCharArray(const std::string& name, unsigned char value, size_t length) { add(name, &value, UCHAR, length); }
    void addDoubleArray(const std::string& name, double value, size_t length) { add(name, &value, DOUBLE, length); }
    void addFloatArray(const std::string& name, double value, size_t length) { add(name, &value, FLOAT, length); }
    void addEnumArray(const std::string& name, EnumMap& enum_m,
                      const std::string& value, size_t length);

    void set(const std::string& name, void* value, size_t size);
    void setBool(const std::string& name, bool value) { set(name, &value, sizeof(bool)); }
    void setLong(const std::string& name, long long value) { set(name, &value, sizeof(long long)); }
    void setInt(const std::string& name, int value) { set(name, &value, sizeof(int)); }
    void setShort(const std::string& name, int value) { set(name, &value, sizeof(short)); }
    void setChar(const std::string& name, int value) { set(name, &value, sizeof(char)); }
    void setULong(const std::string& name, unsigned long long value) { set(name, &value, sizeof(unsigned long long)); }
    void setUInt(const std::string& name, unsigned int value) { set(name, &value, sizeof(unsigned int)); }
    void setUShort(const std::string& name, unsigned int value) { set(name, &value, sizeof(unsigned short)); }
    void setUChar(const std::string& name, unsigned char value) { set(name, &value, sizeof(unsigned char)); }
    void setDouble(const std::string& name, double value) { set(name, &value, sizeof(double)); }
    void setFloat(const std::string& name, double value) { set(name, &value, sizeof(float)); }
    void setEnum(const std::string& name, const std::string& value);
    void setEnum(const std::string& name, const std::string& value, size_t id);
    void setText(const std::string& name, const std::string& value);
    void setText(const std::string& name, const char* value);
    void setObject(const std::string& name, DataObject* value);

    bool hasValue(const std::string& name);
    bool hasObject(const std::string& name);
    bool hasInt(const std::string& name);

  private:
    int _revision;
    int _confno;
    int _id;
    //std::string _name;
    std::string _class;
    std::string _base_class;
    mutable ParamInfoMap _param_m;
    mutable EnumMapMap _enum_m_m;
    mutable ParamNameList _name_v;

  };

}

#endif
