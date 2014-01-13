#ifndef _Belle2_PackageManager_h
#define _Belle2_PackageManager_h

#include "daq/slc/apps/PackageSerializer.h"

#include <daq/slc/dqm/MonitorPackage.h>

#include <daq/slc/system/RWLock.h>

#include <string>

namespace Belle2 {

  class PackageManager {

  public:
    PackageManager(MonitorPackage* monitor);
    virtual ~PackageManager() throw();

  public:
    void setAvailable(bool available);
    bool isAvailable();
    char* createConfig(size_t& buf_size);
    char* createContents(size_t& buf_size);
    char* createContentsAll(size_t& buf_size);
    size_t copyConfig(char* buf, size_t size);
    size_t copyContents(char* buf, size_t size, int& update_id);
    size_t copyContentsAll(char* buf, size_t size, int& update_id);
    const std::string createXML();
    int getUpdateId();
    std::string getName();
    MonitorPackage* getMonitor() { return _monitor; }
    HistoPackage* getPackage() { return _monitor->getPackage(); }
    RootPanel* getPanel() { return _monitor->getRootPanel(); }

  public:
    void clear();
    bool init();
    bool update();

  protected:
    MonitorPackage* _monitor;
    PackageSerializer _serializer;
    bool _available;
    int _updateid;
    Belle2::RWLock _lock;

  };

}

#endif
