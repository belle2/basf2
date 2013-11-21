#ifndef _Belle2_NSMData_hh
#define _Belle2_NSMData_hh

#include "nsm/NSMHandlerException.h"

#include "base/DataObject.h"

extern "C" {
#include "nsm/nsm2.h"
}
#include <map>
#include <vector>

namespace Belle2 {

  class NSMNode;
  class NSMCommunicator;

  enum NSMDataType {
    CHAR = 1, INT16 = 2, INT32 = 4, INT64 = 8,
    BYTE8 = 101, UINT16 = 102, UINT32 = 104, UINT64 = 108,
    FLOAT = 204, DOUBLE = 208, TEXT = 300
  };

  struct NSMDataProperty {
    NSMDataType type;
    size_t length;
    size_t offset;
  };

  typedef std::map<std::string, NSMDataProperty> NSMDataPropertyMap;

  class NSMData {

  public:
    NSMData(const std::string& data_name, const std::string& format,
            int revision) throw() : _pdata(NULL), _data_name(data_name),
      _format(format), _revision(revision) {}
    NSMData() throw() : _pdata(NULL) {}
    virtual ~NSMData() throw() {}

  public:
    const std::string& getName() const { return _data_name; }
    const std::string& getFormat() const { return  _format; }
    int getRevision() const { return _revision; }
    bool isAvailable() throw() { return (_pdata != NULL); }
    void* open(NSMCommunicator* comm) throw(NSMHandlerException);
    void* allocate(NSMCommunicator* comm, int interval = 3) throw(NSMHandlerException);
    void* parse(const char* inc_dir = NULL) throw(NSMHandlerException);
    void* get() throw(NSMHandlerException);
    const void* get() const throw(NSMHandlerException);
    NSMDataPropertyMap& getProperties() { return _pro_m; }
    NSMDataProperty& getProperty(const std::string& label) { return _pro_m[label]; }

  public:
    void* getValue(const std::string& label) throw(NSMHandlerException);
    void* getValueArray(const std::string& label, size_t& length) throw(NSMHandlerException);
    void setValue(const std::string& label, const void* value, size_t size) throw(NSMHandlerException);
    int64 getInt64(const std::string& label) throw(NSMHandlerException);
    int32 getInt32(const std::string& label) throw(NSMHandlerException);
    int16 getInt16(const std::string& label) throw(NSMHandlerException);
    char getChar(const std::string& label) throw(NSMHandlerException);
    uint64 getUInt64(const std::string& label) throw(NSMHandlerException);
    uint32 getUInt32(const std::string& label) throw(NSMHandlerException);
    uint16 getUInt16(const std::string& label) throw(NSMHandlerException);
    byte8 getByte(const std::string& label) throw(NSMHandlerException);
    double getDouble(const std::string& label) throw(NSMHandlerException);
    float getFloat(const std::string& label) throw(NSMHandlerException);
    void setInt64(const std::string& label, int64 value) throw(NSMHandlerException);
    void setInt32(const std::string& label, int32 value) throw(NSMHandlerException);
    void setInt16(const std::string& label, int16 value) throw(NSMHandlerException);
    void setChar(const std::string& label, char value) throw(NSMHandlerException);
    void setUInt64(const std::string& label, uint64 value) throw(NSMHandlerException);
    void setUInt32(const std::string& label, uint32 value) throw(NSMHandlerException);
    void setUInt16(const std::string& label, uint16 value) throw(NSMHandlerException);
    void setByte8(const std::string& label, byte8 value) throw(NSMHandlerException);
    void setDouble(const std::string& label, double value) throw(NSMHandlerException);
    void setFloat(const std::string& label, float value) throw(NSMHandlerException);

    int64* getInt64Array(const std::string& label, size_t& length) throw(NSMHandlerException);
    int32* getInt32Array(const std::string& label, size_t& length) throw(NSMHandlerException);
    int16* getInt16Array(const std::string& label, size_t& length) throw(NSMHandlerException);
    char* getCharArray(const std::string& label, size_t& length) throw(NSMHandlerException);
    uint64* getUInt64Array(const std::string& label, size_t& length) throw(NSMHandlerException);
    uint32* getUInt32Array(const std::string& label, size_t& length) throw(NSMHandlerException);
    uint16* getUInt16Array(const std::string& label, size_t& length) throw(NSMHandlerException);
    byte8* getByteArray(const std::string& label, size_t& length) throw(NSMHandlerException);
    double* getDoubleArray(const std::string& label, size_t& length) throw(NSMHandlerException);
    float* getFloatArray(const std::string& label, size_t& length) throw(NSMHandlerException);

    const std::string toSQLConfig();
    const std::string toSQLNames();
    const std::string toSQLValues();
    void setSQLValues(std::vector<std::string>& name_v,
                      std::vector<std::string>& value_v);
    void getSQLValues(std::vector<std::string>& name_v,
                      std::vector<std::string>& value_v);
    DataObject* createDataObject();
    const std::string toXML();

  private:
    int initProperties() throw();

  private:
    void* _pdata;
    std::string _data_name;
    std::string _format;
    int _revision;
    NSMDataPropertyMap _pro_m;
    std::vector<std::string> _label_v;

  };

}

#endif
