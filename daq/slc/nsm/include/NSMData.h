#ifndef _Belle2_NSMData_hh
#define _Belle2_NSMData_hh

#include "daq/slc/nsm/NSMHandlerException.h"

#include <daq/slc/database/DBObject.h>

#include <daq/slc/base/StringUtil.h>

extern "C" {
#include "nsm2/nsm2.h"
#if NSM_PACKAGE_VERSION >= 1914
#include "nsm2/nsmparse.h"
#endif
}
#include <map>
#include <vector>

namespace Belle2 {

  class NSMCommunicator;

  class NSMData : public DBObject {

  public:
    typedef std::vector<NSMData> NSMDataList;
    typedef std::map<std::string, NSMDataList> NSMDataListMap;

  public:
    NSMData(const std::string& dataname,
            const std::string& format, int revision) throw();
    NSMData() throw();
    NSMData(const NSMData& data) throw();
    virtual ~NSMData() throw();

  private:
    NSMData(void* pdata, const NSMData& data) throw();
    NSMData(void* pdata, const std::string& dataname,
            const std::string& format, int revision) throw();

  public:
    const std::string& getFormat() const throw() { return getTable(); }
    int getSize() const throw() { return m_size; }
    void setFormat(const std::string& format) throw() { setTable(format); }
    void setSize(int size) throw() { m_size = size; }
    bool isAvailable() throw() { return (m_pdata != NULL); }
    void* open(NSMCommunicator* comm) throw(NSMHandlerException);
    void* allocate(NSMCommunicator* comm, int interval = 3)
    throw(NSMHandlerException);
    void* parse(const char* inc_dir = NULL, bool allocated = false)
    throw(NSMHandlerException);
    void* get() throw() { return m_pdata; }
    const void* get() const throw() { return m_pdata; }
    const NSMDataList& getObjects(const std::string& name) const throw();
    NSMDataList& getObjects(const std::string& name) throw();

  public:
    virtual void reset() throw();
    virtual void* getValue(const std::string& name) throw();
    virtual const void* getValue(const std::string& name) const throw();
    virtual void setValue(const std::string& name, const void* value, int size) throw();
    virtual void addValue(const std::string& name, const void* value,
                          FieldInfo::Type type, int length = 0) throw();
    virtual int getNObjects(const std::string& name) const throw();
    virtual const DBObject& getObject(const std::string& name, int index = 0) const throw();
    virtual DBObject& getObject(const std::string& name, int index = 0) throw();
    virtual const std::string getText(const std::string&) const throw() { return ""; }
    virtual void addText(const std::string&, const std::string&) throw() {}
    virtual void addEnum(const std::string&, const std::string&) throw() {}
    virtual void setObject(const std::string&, int, const Belle2::DBObject&) throw() {}
    virtual const std::string getEnum(const std::string&) const throw() { return ""; }


  public:
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  private:
    int initProperties() throw();
#if NSM_PACKAGE_VERSION >= 1914
    NSMparse* parse(NSMparse* ptr, int& length,
                    std::string& name_in) throw(NSMHandlerException);
#endif

  private:
    bool m_allocated;
    void* m_pdata;
    int m_size;
    int m_offset;
    mutable NSMDataListMap m_data_v_m;

  };

}

#endif
