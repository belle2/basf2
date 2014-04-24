#ifndef _Belle2_PackageBuffer_hh
#define _Belle2_PackageBuffer_hh

#include <daq/slc/dqm/HistoPackage.h>

#include <daq/slc/system/RWLock.h>
#include <daq/slc/system/ZipDeflater.h>
#include <daq/slc/system/StreamSizeCounter.h>

namespace Belle2 {

  class PackageBuffer {

  public:
    PackageBuffer();
    virtual ~PackageBuffer() throw();

  public:
    size_t allocate(HistoPackage* pack);
    void update();
    size_t size();
    char* createBuffer(size_t& buf_size);
    size_t copy(char* buf, size_t buf_size);

  protected:
    virtual void serialize(Belle2::Writer& writer) = 0;
    virtual void serializeAll(Belle2::Writer& writer) {
      serialize(writer);
    }

  protected:
    HistoPackage* _pack;
    Belle2::BufferedWriter* _writer;
    //Belle2::ZipDeflater* _writer;
    Belle2::RWLock _lock;

  };

  class PackageConfigBuffer : public PackageBuffer {

  public:
    PackageConfigBuffer() {}
    virtual ~PackageConfigBuffer() throw() {}

  protected:
    virtual void serialize(Belle2::Writer& writer) {
      _pack->writeConfig(writer);
    }

  };

  class PackageContentsBuffer : public PackageBuffer {

  public:
    PackageContentsBuffer() {}
    virtual ~PackageContentsBuffer() throw() {}

  protected:
    virtual void serialize(Belle2::Writer& writer) {
      _pack->writeContents(writer);
    }
    virtual void serializeAll(Belle2::Writer& writer) {
      _pack->writeContents(writer, true);
    }

  };

  class PackageContentsAllBuffer : public PackageBuffer {

  public:
    PackageContentsAllBuffer() {}
    virtual ~PackageContentsAllBuffer() throw() {}

  protected:
    virtual void serialize(Belle2::Writer& writer) {
      _pack->writeContents(writer, true);
    }

  };

};

#endif
