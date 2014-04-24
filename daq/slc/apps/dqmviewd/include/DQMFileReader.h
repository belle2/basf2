#ifndef _Belle2_DQMFileReader_hh
#define _Belle2_DQMFileReader_hh

#include "daq/slc/apps/dqmviewd/DQMFileReader.h"

#include <daq/slc/dqm/DQMHistMap.h>

#include <daq/slc/system/Mutex.h>

#include <TH1.h>
#include <TMapFile.h>

namespace Belle2 {

  class DQMFileReader {

  public:
    DQMFileReader(const std::string& name,
                  const std::string& file_path)
      : _name(name), _hist_m(file_path), _file(NULL),
        _ready(false), _updateid(0) {}
    ~DQMFileReader();

  public:
    bool init();
    int update();
    int getUpdateId() { return _updateid; }
    const std::string& getName() { return _name; }
    const std::string& getFileName() { return _hist_m.getFileName(); }
    TH1* getHist(const std::string& name);
    TH1Map& getHists() { return _hist_m.getHists(); }
    const TH1Map& getHists() const { return _hist_m.getHists(); }
    bool isReady() const { return _ready; }
    bool dump(const std::string& dir,
              unsigned int expno, unsigned int runno);
    void lock() { _mutex.lock(); }
    void unlock() { _mutex.unlock(); }

  private:
    std::string _name;
    DQMHistMap _hist_m;
    TMapFile* _file;
    bool _ready;
    int _updateid;
    Mutex _mutex;

  };

}

#endif
