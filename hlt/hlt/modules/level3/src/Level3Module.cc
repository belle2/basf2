/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hidekazu Kakuno                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "hlt/hlt/modules/level3/Level3Module.h"
#include "hlt/hlt/modules/level3/FTFinder.h"
#include "hlt/hlt/modules/level3/FCFinder.h"
#include "hlt/hlt/modules/level3/FCCluster.h"


#include "hlt/hlt/modules/level3/L3TrackTrigger.h"
#include "hlt/hlt/modules/level3/L3EnergyTrigger.h"

using namespace Belle2;

REG_MODULE(Level3)

Level3Module::Level3Module()
  : L3TriggerManager(),
    Module(),
    m_tFinder(FTFinder::instance()),
    m_cFinder(FCFinder::instance())
{
  // confirmed the result of parallel processing on Oct.13 2014
  setPropertyFlags(c_ParallelProcessingCertified);

  setDescription("Level3 is the Level-3 software trigger module.");

  //---
  // Adding Trigger algorithms
  //  - Bit assigment of HLTTag::m_HLTAlgoInfo[Level3] and L3Tag::m_SummaryWord
  //    corresponds to the order of addTrigger() call
  //---
  // Bit 1
  L3TrackTrigger* TrackTrigger = new L3TrackTrigger;
  addTrigger(TrackTrigger);
  // Bit 2
  L3EnergyTrigger* EnergyTrigger = new L3EnergyTrigger;
  addTrigger(EnergyTrigger);

  //---
  // Adding Parameters
  //---
  addParam("dr_cut", TrackTrigger->drCut(),
           "Minumum |dr| value of a reconstructed CDC track", 1.0);
  addParam("dz_cut", TrackTrigger->dzCut(),
           "Minumum |dz| value of a reconstructed CDC track", 4.0);
  addParam("pt_cut", TrackTrigger->ptCut(),
           "Minumum transverse momentum of a reconstructed CDC track", 0.3);
  addParam("es_cut", m_EsCut,
           "Minumum energy of the seed for a reconstructed ECL cluster", 0.01);
  addParam("ec_cut", m_EcCut,
           "Minumum energy of a reconstructed ECL cluster", 0.02);
  addParam("min_n_trks", TrackTrigger->minNGoodTrks(),
           "Minumum number of good CDC tracks", 1);
  addParam("min_energy", EnergyTrigger->minEnergy(),
           "Minumum threshold of the energy sum of ECL clusters", 4.0);
  addParam("find_evt_vtx", m_findEvtVtx,
           "Flag for event vertex finding", 0);
  addParam("save_data", m_saveData,
           "=0:HLTTag only, =1:HLTTag+L3Tag, =2:HLTTag+L3Tag+L3Track+L3Cluster", 2);

}

Level3Module::~Level3Module()
{
  delete &m_tFinder;
  delete &m_cFinder;
}

void Level3Module::initialize()
{
  m_tFinder.init();
  m_cFinder.init();
  if (m_saveData >= 2) {
    m_l3Track.registerInDataStore();
    m_l3Cluster.registerInDataStore();
  }
  initializeTriggerManager();
  printInformation();
}

void Level3Module::beginRun()
{
  m_tFinder.beginRun();
  m_cFinder.beginRun();
}

void Level3Module::event()
{
  setReturnValue(false);

  //---
  // Preselect events before reconstruction
  //---
  if (preselect()) {
    setReturnValue(true);
    return;
  }

  //---
  // Reconstruct CDC tracks ECL clusters
  //---
  reconstruct();

  //--
  // select events after reconstruction
  //--
  if (select()) {
    setReturnValue(true);
  }
}

void Level3Module::terminate()
{
  m_tFinder.term();
  m_cFinder.term();
}

void Level3Module::reconstruct()
{
  // reconstruct ECL clusters
  m_cFinder.event(m_EsCut, m_EcCut);

  // reconstruct CDC tracks
  m_tFinder.event(m_findEvtVtx);

  if (m_saveData >= 2) {
    // make an array of L3Cluster
    StoreArray<L3Cluster> clusters;
    clusters.clear();
    FTList<FCCluster*>& clstrs = m_cFinder.getClusters();
    for (int i = 0; i < clstrs.length(); i++) {
      TVector3 pos = clstrs[i]->getPosition();
      double e = clstrs[i]->getEnergy();
      clusters.appendNew(L3Cluster(e, pos));
    }

    // make an array of L3Track
    StoreArray<L3Track> tracks;
    tracks.clear();
    FTList<FTTrack*>& trks = m_tFinder.getTracks();
    for (int i = 0; i < trks.length(); i++) {
      const TVectorD& h = trks[i]->getHelix();
      tracks.appendNew(L3Track(h));
    }
  }
}
