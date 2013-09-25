#ifndef _B2DQM_HistoFactory_hh
#define _B2DQM_HistoFactory_hh

#include "Histo.hh"

#include <util/Reader.hh>

#include <string>

namespace B2DQM {

  namespace HistoFactory {
    
    MonObject* create(const std::string& type);
    MonObject* create(B2DAQ::Reader& reader) throw(B2DAQ::IOException);
    MonObject* createFull(B2DAQ::Reader& reader) throw(B2DAQ::IOException);

  };

};

#endif
