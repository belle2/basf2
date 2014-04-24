#ifndef _Belle2_HistoFactory_hh
#define _Belle2_HistoFactory_hh

#include "daq/slc/dqm/Histo.h"

#include "daq/slc/base/Reader.h"

#include <string>

namespace Belle2 {

  namespace HistoFactory {

    MonObject* create(const std::string& type);
    MonObject* create(Belle2::Reader& reader) throw(Belle2::IOException);
    MonObject* createFull(Belle2::Reader& reader) throw(Belle2::IOException);

  };

};

#endif
