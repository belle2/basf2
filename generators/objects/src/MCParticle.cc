/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>

#include <TDatabasePDG.h>

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/StoreArray.h>
#include <generators/objects/MCParticle.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//Define exceptions
namespace Belle2 {
  BELLE2_DEFINE_EXCEPTION(LastChildIndexOutOfRangError, "Last child index out of range !");
  BELLE2_DEFINE_EXCEPTION(NoParticleListSetError, "No Particle list set, cannot determine related particles !");
}

void MCParticle::setPDG(int pdg)
{
  m_pdg = pdg;
  m_mass = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
}


const vector<MCParticle*> MCParticle::getDaughters() const
{
  vector<MCParticle*> result;
  if (m_first_daughter > 0) {
    fixParticleList();
    if (m_last_daughter > m_plist->GetEntriesFast()) throw LastChildIndexOutOfRangError();
    TClonesArray &plist = *m_plist;
    result.reserve(m_last_daughter - m_first_daughter + 1);
    for (int i = m_first_daughter - 1; i < m_last_daughter; i++) {
      result.push_back(static_cast<MCParticle*>(plist[i]));
    }
  }
  return result;
}


const vector<MCParticle*> MCParticle::getMothers() const
{
  //TODO: Maybe add transient vector of mothers and childrens to MCParticle and fill them only once?
  vector<MCParticle*> result;
  fixParticleList();
  //We only look up to m_index since MCParticles are sorted breadth first
  for (int i = 0; i < m_index - 1; ++i) {
    MCParticle &mc = *(static_cast<MCParticle*>(m_plist->At(i)));
    if (mc.m_first_daughter <= m_index && m_index <= mc.m_last_daughter) {
      result.push_back(&mc);
    }
  }
  return result;
}


void MCParticle::fixParticleList() const
{
  if (m_plist != 0) return;

  TClonesArray* plist(0);
  //Search default location
  //
  StoreArray<MCParticle> MCParticles(DEFAULT_MCPARTICLES);
  if (MCParticles->IndexOf(this) >= 0) {
    plist = MCParticles.getPtr();
  } else {
    //Search all StoreArrays which happen to store MCParticles
    //FIXME: access to m_map which should be protected, but StoreIter seemed broken
    StoreArrayMap  &map = *(DataStore::Instance().getArrayIterator(c_Event)->m_map);
    for (StoreArrayMap::iterator it = map.begin(); it != map.end(); it++) {
      TClonesArray &value = *(static_cast<TClonesArray*>(it->second));
      if (value.GetClass() == Class() && value.IndexOf(this) >= 0) {
        plist = &value;
        break;
      }
    }
  }
  //Could not find any collection, raise exception
  if (!plist) {
    ERROR("Could not determine StoreArray the MCParticle belongs to !");
    throw NoParticleListSetError();
  }
  //Set plist pointer and index for whole array
  for (int i = 0; i < plist->GetEntries(); i++) {
    MCParticle &mc = *(static_cast<MCParticle*>(plist->At(i)));
    mc.m_plist = plist;
    mc.m_index = i + 1;
  }
}

ClassImp(MCParticle)
