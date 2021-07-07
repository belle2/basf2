/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMData_hh
#define _Belle2_NSMData_hh

#include <daq/slc/base/AbstractDBObject.h>

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
            const std::string& format, int revision);
    NSMData();
    NSMData(const NSMData& data);
    virtual ~NSMData();

  private:
    NSMData(void* pdata, const NSMData& data);
    NSMData(void* pdata, const std::string& dataname,
            const std::string& format, int revision);

  public:
    int getRevision() const { return m_revision; }
    const std::string& getFormat() const { return m_format; }
    int getSize() const { return m_size; }
    void setRevision(int revision) { m_revision = revision; }
    void setFormat(const std::string& format) { m_format = format; }
    void setSize(int size) { m_size = size; }
    bool isAvailable() { return (m_pdata != NULL); }
    void* open(NSMCommunicator& comm);
    void* allocate(NSMCommunicator& comm, int interval = 2);
    void flush();
    void* get() { return m_pdata; }
    const void* get() const { return m_pdata; }
    void* parse(const char* inc_dir = NULL, bool allocated = false);

    int getNObjects(const std::string& name) const;
    const NSMDataList& getObjects(const std::string& name) const;
    NSMDataList& getObjects(const std::string& name);
    const NSMData& getObject(const std::string& name, int index = 0) const;
    NSMData& getObject(const std::string& name, int index = 0);

    void print(const std::string& name_in = "") const;
    void printPV(const std::string& name_in = "") const;
    const void* find(const std::string& name_in, DBField::Type& type, int& length) const;
    void search(NameValueList& map, const std::string& name = "") const;

  public:
    virtual void* getValue(const std::string& name);
    virtual const void* getValue(const std::string& name) const;
    virtual void setValue(const std::string& name, const void* value, int size);
    virtual void addValue(const std::string& name, const void* value,
                          DBField::Type type, int length = 0);
    virtual const std::string& getText(const std::string&) const { return m_empty; }
    virtual void addText(const std::string&, const std::string&) {}

  public:
    virtual void readObject(Reader& reader);
    virtual void writeObject(Writer& writer) const;

  protected:
    virtual void reset();

  private:
    void set(void* pdata) { m_pdata = pdata; }
    int initProperties();
#if NSM_PACKAGE_VERSION >= 1914
    NSMparse* parse(NSMparse* ptr, int& length,
                    std::string& name_in);
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
