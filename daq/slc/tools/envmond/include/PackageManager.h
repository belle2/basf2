#ifndef _Belle2_PackageManager_hh
#define _Belle2_PackageManager_hh

#include "PackageSerializer.h"
#include "dqm/AbstractMonitor.h"

#include <dqm/HistoPackage.h>
#include <dqm/RootPanel.h>

#include <system/RWLock.h>

#include <string>

namespace Belle2 {

  class PackageManager {

  public:
    PackageManager(NSMData* data, AbstractMonitor* monitor);
    virtual ~PackageManager() throw();

  public:
    void clear();
    void init();
    void update();

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

  protected:
    NSMData* _data;
    AbstractMonitor* _monitor;
    PackageSerializer _serializer;
    bool _available;
    RWLock _lock;

  };

}

#endif
