#ifndef _Belle2_HistoFactory_h
#define _Belle2_HistoFactory_h

#include "daq/slc/dqm/Histo.h"

#include "daq/slc/base/Reader.h"

#include <string>

namespace Belle2 {

  namespace HistoFactory {

    MonObject* create(const std::string& type);
    MonObject* create(Reader& reader) throw(IOException);
    MonObject* createFull(Reader& reader) throw(IOException);

  };

};

#endif
