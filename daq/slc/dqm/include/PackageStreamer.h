#ifndef _Belle2_PackageStreamer_h
#define _Belle2_PackageStreamer_h

#include <system/Writer.h"
#include <system/Reader.h"

#include "daq/slc/dqm/HistoPackage.h"

namespace Belle2 {

  class PackageStreamer {

  public:
    PackageStreamer() {}
    ~PackageStreamer() throw() {}

  public:
    void writeConfig(const HistoPackage& pack, Writer& writer) throw(IOException);
    void writeHistory(const HistoPackage& pack, Writer& writer) throw(IOException);
    void writeUpdate(const HistoPackage& pack, Writer& writer) throw(IOException);
    void readConfig(HistoPackage& pack, Reader& reader) throw(IOException);
    void readHistory(HistoPackage& pack, Reader& reader) throw(IOException);
    void readUpdate(HistoPackage& pack, Reader& reader) throw(IOException);

  };

};

#endif
