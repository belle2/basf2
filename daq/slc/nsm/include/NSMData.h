#ifndef _Belle2_NSMData_hh
#define _Belle2_NSMData_hh

#include "daq/slc/nsm/NSMHandlerException.h"

#include <daq/slc/base/AbstractDBObject.h>
#include <daq/slc/base/StringUtil.h>

extern "C" {
#include <nsm2/nsm2.h>
#if NSM_PACKAGE_VERSION >= 1914
#include <nsm2/nsmparse.h>
#endif
}

#include <map>
#include <vector>

namespace Belle2 {

  class NSMCommunicator;

  class NSMData : public AbstractDBObject {

    typedef std::vector<NSMData> NSMDataList;
    typedef std::map<std::string, NSMDataList> NSMDataListMap;

  public:
    struct NameValue {
      std::string name;
      std::string value;
      void* buf;
      DBField::Type type;
    };
    typedef std::vector<NameValue> NameValueList;

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
    int getRevision() const throw() { return m_revision; }
    const std::string& getFormat() const throw() { return m_format; }
    int getSize() const throw() { return m_size; }
    void setRevision(int revision) throw() { m_revision = revision; }
    void setFormat(const std::string& format) throw() { m_format = format; }
    void setSize(int size) throw() { m_size = size; }
    bool isAvailable() throw() { return (m_pdata != NULL); }
    void* open(NSMCommunicator& comm) throw(NSMHandlerException);
    void* allocate(NSMCommunicator& comm, int interval = 2) throw(NSMHandlerException);
    void flush() throw(NSMHandlerException);
    void* get() throw() { return m_pdata; }
    const void* get() const throw() { return m_pdata; }
    void* parse(const char* inc_dir = NULL, bool allocated = false)
    throw(NSMHandlerException);

    int getNObjects(const std::string& name) const throw();
    const NSMDataList& getObjects(const std::string& name) const throw(std::out_of_range);
    NSMDataList& getObjects(const std::string& name) throw(std::out_of_range);
    const NSMData& getObject(const std::string& name, int index = 0) const throw(std::out_of_range);
    NSMData& getObject(const std::string& name, int index = 0) throw(std::out_of_range);

    void print(const std::string& name_in = "") const throw();
    void printPV(const std::string& name_in = "") const throw();
    const void* find(const std::string& name_in, DBField::Type& type, int& length) const throw();
    void search(NameValueList& map, const std::string& name = "") const throw();

  public:
    virtual void* getValue(const std::string& name) throw(std::out_of_range);
    virtual const void* getValue(const std::string& name) const throw(std::out_of_range);
    virtual void setValue(const std::string& name, const void* value, int size) throw();
    virtual void addValue(const std::string& name, const void* value,
                          DBField::Type type, int length = 0) throw();
    virtual const std::string& getText(const std::string&) const throw() { return m_empty; }
    virtual void addText(const std::string&, const std::string&) throw() {}

  public:
    virtual void readObject(Reader& reader) throw(IOException);
    virtual void writeObject(Writer& writer) const throw(IOException);

  protected:
    virtual void reset() throw();

  private:
    void set(void* pdata) throw() { m_pdata = pdata; }
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
    std::string m_format;
    int m_revision;
    NSMDataListMap m_data_v_m;
    std::string m_empty;
    NSMCommunicator* m_com;
    int m_tstamp;

  };

  typedef std::vector<NSMData> NSMDataList;
  typedef std::map<std::string, NSMDataList> NSMDataListMap;

}

#endif
