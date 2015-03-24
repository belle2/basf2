#ifndef _Belle2_DQMFileReader_hh
#define _Belle2_DQMFileReader_hh

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/dqm/DQMHistMap.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/base/StringUtil.h>

#include <TH1.h>
#include <TMapFile.h>

namespace Belle2 {

  class DQMViewCallback;

  class DQMFileReader {

  public:
    DQMFileReader(const std::string& name,
                  const std::string& file_path)
      : m_name(name), m_hist_m(file_path), m_file(NULL),
        m_ready(false), m_updateid(0) {}
    ~DQMFileReader();

  public:
    bool init(int index, DQMViewCallback& callback);
    int update();
    int getUpdateId() { return m_updateid; }
    const std::string& getName() { return m_name; }
    const std::string& getFileName() { return m_hist_m.getFileName(); }
    TH1* getHist(const std::string& name);
    TH1Map& getHists() { return m_hist_m.getHists(); }
    const TH1Map& getHists() const { return m_hist_m.getHists(); }
    const std::string& getDirectory(const std::string& name)
    {
      return m_hist_m.getDirectory(name);
    }
    bool isReady() const { return m_ready; }
    bool dump(const std::string& dir,
              unsigned int expno, unsigned int runno);
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    StringList& getHistNames() { return m_name_v; }

  private:
    std::string m_name;
    StringList m_name_v;
    DQMHistMap m_hist_m;
    TMapFile* m_file;
    bool m_ready;
    int m_updateid;
    Mutex m_mutex;

  };

}

#endif
