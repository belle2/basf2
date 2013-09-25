#ifndef _B2DQM_PackageBuffer_hh
#define _B2DQM_PackageBuffer_hh

#include <dqm/HistoPackage.hh>

#include <system/RWLock.hh>
#include <system/ZipDeflater.hh>
#include <system/StreamSizeCounter.hh>

namespace B2DQM {

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
    virtual void serialize(B2DAQ::Writer& writer) = 0;
    virtual void serializeAll(B2DAQ::Writer& writer) {
      serialize(writer);
    }

  protected:
    HistoPackage* _pack;
    B2DAQ::ZipDeflater* _writer;
    B2DAQ::RWLock _lock;

  };

  class PackageConfigBuffer : public PackageBuffer {

  public:
    PackageConfigBuffer() {}
    virtual ~PackageConfigBuffer() throw() {}

  protected:
    virtual void serialize(B2DAQ::Writer& writer) {
      _pack->writeConfig(writer);
    }

  };

  class PackageContentsBuffer : public PackageBuffer {

  public:
    PackageContentsBuffer() {}
    virtual ~PackageContentsBuffer() throw() {}

  protected:
    virtual void serialize(B2DAQ::Writer& writer) {
      _pack->writeContents(writer);
    }
    virtual void serializeAll(B2DAQ::Writer& writer) {
      _pack->writeContents(writer, true);
    }

  };

  class PackageContentsAllBuffer : public PackageBuffer {

  public:
    PackageContentsAllBuffer() {}
    virtual ~PackageContentsAllBuffer() throw() {}

  protected:
    virtual void serialize(B2DAQ::Writer& writer) {
      _pack->writeContents(writer, true);
    }

  };

};

#endif
