#ifndef _Belle2_PackageStreamer_hh
#define _Belle2_PackageStreamer_hh

#include <system/Writer.h"
#include <system/Reader.h"

#include "dqm/HistoPackage.h"

namespace Belle2 {

  class PackageStreamer {

  public:
    PackageStreamer() {}
    ~PackageStreamer() throw() {}

  public:
    void writeConfig(const HistoPackage& pack, Belle2::Writer& writer) throw(Belle2::IOException);
    void writeHistory(const HistoPackage& pack, Belle2::Writer& writer) throw(Belle2::IOException);
    void writeUpdate(const HistoPackage& pack, Belle2::Writer& writer) throw(Belle2::IOException);
    void readConfig(HistoPackage& pack, Belle2::Reader& reader) throw(Belle2::IOException);
    void readHistory(HistoPackage& pack, Belle2::Reader& reader) throw(Belle2::IOException);
    void readUpdate(HistoPackage& pack, Belle2::Reader& reader) throw(Belle2::IOException);

  };

};

#endif
