#ifndef _Belle2_DQMViewMaster_h
#define _Belle2_DQMViewMaster_h

#include "daq/slc/apps/MonitorMaster.h"

#include <daq/slc/dqm/DQMPackage.h>

#include <daq/slc/system/Fork.h>

#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  class DQMViewMaster : public MonitorMaster {

  public:
    void add(const std::string mapname, int port,
             DQMPackage* monitor) {
      _mapname_v.push_back(mapname);
      _port_v.push_back(port);
      addManager(new PackageManager(monitor));
    }
    bool boot();
    bool abort();
    void setRunNumbers(unsigned int expno, unsigned int runno) {
      _expno = expno;
      _runno = runno;
    }
    unsigned int getExpNumber() const {  return _expno; }
    unsigned int getRunNumber() const {  return _runno; }

  private:
    unsigned int _expno;
    unsigned int _runno;
    std::vector<std::string> _mapname_v;
    std::vector<int> _port_v;
    std::vector<Fork> _fork_v;

  };

  class HSeverExecutor {

  public:
    HSeverExecutor(int port, const std::string& mapfile,
                   const std::string& dir)
      : _port(port), _mapfile(mapfile), _dir(dir) {}

  public:
    void run();

  private:
    int _port;
    std::string _mapfile;
    std::string _dir;

  };

}

#endif
