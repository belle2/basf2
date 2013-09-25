#ifndef _B2DQM_PackageStreamer_hh
#define _B2DQM_PackageStreamer_hh

#include <system/Writer.hh>
#include <system/Reader.hh>

#include "HistoPackage.hh"

namespace B2DQM {

  class PackageStreamer {

  public:
    PackageStreamer() {}
    ~PackageStreamer() throw() {}

  public:
    void writeConfig(const HistoPackage& pack, B2DAQ::Writer& writer) throw(B2DAQ::IOException);
    void writeHistory(const HistoPackage& pack,B2DAQ::Writer& writer) throw(B2DAQ::IOException);
    void writeUpdate(const HistoPackage& pack, B2DAQ::Writer& writer) throw(B2DAQ::IOException);
    void readConfig(HistoPackage& pack, B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    void readHistory(HistoPackage& pack, B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    void readUpdate(HistoPackage& pack, B2DAQ::Reader& reader) throw(B2DAQ::IOException);

  };

};

#endif
