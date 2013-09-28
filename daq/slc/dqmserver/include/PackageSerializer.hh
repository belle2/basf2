#ifndef _B2DQM_PackageSerializer_hh
#define _B2DQM_PackageSerializer_hh

#include "PackageBuffer.hh"

#include <dqm/HistoPackage.hh>

namespace B2DQM {

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
