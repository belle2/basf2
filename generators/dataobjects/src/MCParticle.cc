/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
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
#include <generators/dataobjects/MCParticle.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;

//Define exceptions
namespace Belle2 {
  /** Exception is thrown if the requested index for the last child is out of range. */
  BELLE2_DEFINE_EXCEPTION(LastChildIndexOutOfRangError, "Last child index out of range !");
  /** Exception is thrown if no pointer to the particle list was set. */
  BELLE2_DEFINE_EXCEPTION(NoParticleListSetError, "No Particle list set, cannot determine related particles !");
  /** Exception is thrown if the pdg value of the MCParticle is not known to the internal database (TDatabasePDG). */
  BELLE2_DEFINE_EXCEPTION(ParticlePDGNotKnownError, "The pdg value (%1%) of the MCParticle is not known !");
}


void MCParticle::setPDG(int pdg)
{
  m_pdg = pdg;
  if (TDatabasePDG::Instance()->GetParticle(pdg) == NULL) throw(ParticlePDGNotKnownError() << pdg);
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



void MCParticle::fixParticleList() const
{
  if (m_plist != 0) return;

  TClonesArray* plist(0);

  //Search default location
  StoreArray<MCParticle> MCParticles(DEFAULT_MCPARTICLES);
  if (MCParticles->IndexOf(this) >= 0) {
    plist = MCParticles.getPtr();
  } else {
    //Search all StoreArrays which happen to store MCParticles
    StoreMapIter<StoreArrayMap>* iter = DataStore::Instance().getArrayIterator(c_Event);
    for (iter->first(); iter->isDone(); iter++) {
      TClonesArray &value = *(static_cast<TClonesArray*>(iter->value()));
      if (value.GetClass() == Class() && value.IndexOf(this) >= 0) {
        plist = &value;
        break;
      }
    }
  }
  //Could not find any collection, raise exception
  if (!plist) {
    B2ERROR("Could not determine StoreArray the MCParticle belongs to !");
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
