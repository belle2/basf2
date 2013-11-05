#ifndef _Belle2_PackageSerializer_hh
#define _Belle2_PackageSerializer_hh

#include "PackageBuffer.h"

#include <dqm/HistoPackage.h>

namespace Belle2 {

  class PackageSerializer {

  public:
    PackageSerializer() {}
    virtual ~PackageSerializer() throw() {}

  public:
    bool allocate(HistoPackage* pack);
    int update();
    PackageBuffer& getConfig() { return _config; }
    PackageBuffer& getContents() { return _contents; }
    PackageBuffer& getContentsAll() { return _contents_all; }

  private:
    HistoPackage* _pack;
    PackageConfigBuffer _config;
    PackageContentsBuffer _contents;
    PackageContentsAllBuffer _contents_all;

  };

}

#endif
