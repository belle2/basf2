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

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;


void MCParticle::setMassFromPDG()
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdg) == NULL) throw(ParticlePDGNotKnownError() << m_pdg);
  m_mass = TDatabasePDG::Instance()->GetParticle(m_pdg)->Mass();
}

void MCParticle::setChargeFromPDG()
{
  if (TDatabasePDG::Instance()->GetParticle(m_pdg) == NULL) throw(ParticlePDGNotKnownError() << m_pdg);
  m_charge = 3 * TDatabasePDG::Instance()->GetParticle(m_pdg)->Charge();
}



const vector<MCParticle*> MCParticle::getDaughters() const
{
  vector<MCParticle*> result;
  if (m_firstDaughter > 0) {
    fixParticleList();
    if (m_lastDaughter > m_plist->GetEntriesFast()) throw LastChildIndexOutOfRangError();
    TClonesArray &plist = *m_plist;
    result.reserve(m_lastDaughter - m_firstDaughter + 1);
    for (int i = m_firstDaughter - 1; i < m_lastDaughter; i++) {
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
    StoreMapIter<DataStore::StoreArrayMap>* iter = DataStore::Instance().getArrayIterator(DataStore::c_Event);
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
