#ifndef _B2DAQ_NSMData_hh
#define _B2DAQ_NSMData_hh

#include "NSMHandlerException.hh"

#include <map>
#include <vector>

namespace B2DAQ {

  class NSMNode;

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
    void* open() throw(NSMHandlerException);
    void* allocate(int interval = 3) throw(NSMHandlerException);
    void* get() throw(NSMHandlerException);
    const void* get() const throw(NSMHandlerException);
    NSMDataPropertyMap& getProperties() { return _pro_m; }
    NSMDataProperty& getProperty(const std::string& label) { return _pro_m[label]; }

    const std::string toSQLConfig();
    const std::string toSQLNames();
    const std::string toSQLValues();
    void setSLQValues(std::vector<std::string>& name_v,
		      std::vector<std::string>& value_v);

  private:
    void initProperties() throw();

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
