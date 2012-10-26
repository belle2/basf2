#ifndef EvtBelle2Models_H
#define EvtBelle2Models_H

#include <list>

#include "EvtGenBase/EvtDecayBase.hh"

// Add official BelleII EvtGen model headers here
#include "generators/evtgen/models/EvtB0toKsKK.h"

void RegBelle2Models(std::list<EvtDecayBase*>& belle2Models)
{

  // Add instance of official BelleII EvtGen model here
  belle2Models.push_back(new EvtB0toKsKK);

  return;
}

#endif
