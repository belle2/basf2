/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleRecoStatsTool.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

using namespace Belle2;
using namespace std;

void NtupleRecoStatsTool::setupTree()
{
  m_iClusters = -1;
  m_iNeutralClusters = -1;
  m_iChargedClusters = -1;
  m_iGoodNeutralClusters = -1;
  m_iTracks = -1;
  m_iMCParticles = -1;
  m_iParticles = -1;
  m_neutralECLEnergy = 0.0;
  m_chargedECLEnergy = 0.0;
  m_goodNeutralECLEnergy = 0.0;
  m_tree->Branch("nECLClusters",            &m_iClusters,            "nECLClusters/I");
  m_tree->Branch("nNeutralECLClusters",     &m_iNeutralClusters,     "nNeutralECLClusters/I");
  m_tree->Branch("nChargedECLClusters",     &m_iChargedClusters,     "nChargedECLClusters/I");
  m_tree->Branch("nGoodNeutralECLClusters", &m_iGoodNeutralClusters,  "nGoodNeutralECLClusters/I");
  m_tree->Branch("neutralECLEnergy",        &m_neutralECLEnergy,     "neutralECLEnergy/F");
  m_tree->Branch("chargedECLEnergy",        &m_chargedECLEnergy,     "chargedECLEnergy/F");
  m_tree->Branch("goodNeutralECLEnergy",    &m_goodNeutralECLEnergy, "goodNeutralECLEnergy/F");
  m_tree->Branch("nTracks" ,      &m_iTracks,      "nTracks/I");
  m_tree->Branch("nMCParticles",  &m_iMCParticles, "nMCParticles/I");
  m_tree->Branch("nParticles" ,   &m_iParticles,   "nParticles/I");

}

void NtupleRecoStatsTool::eval(const  Particle*)
{
  m_iClusters = 0;
  m_iNeutralClusters = 0;
  m_iChargedClusters = 0;
  m_iGoodNeutralClusters = 0;
  m_iTracks = 0;
  m_iMCParticles = 0;
  m_iParticles = 0;
  m_neutralECLEnergy = 0.0;
  m_chargedECLEnergy = 0.0;
  m_goodNeutralECLEnergy = 0.0;

  StoreArray<ECLCluster>    ECLClusters;
  m_iClusters = (int) ECLClusters.getEntries();

  for (int i = 0; i < ECLClusters.getEntries(); i++) {
    const ECLCluster* cluster      = ECLClusters[i];

    if (cluster->isNeutral()) {
      m_neutralECLEnergy += cluster->getEnergy();
      m_iNeutralClusters++;

      // is it good cluster?
      double energy = cluster->getEnergy();
      double e9e25  = cluster->getE9oE25();
      int    region = 0;
      //double timing = cluster->getTiming();

      float theta = cluster->getTheta();
      if (theta < 0.555) {
        region = 1.0;
      } else if (theta < 2.26) {
        region = 2.0;
      } else {
        region = 3.0;
      }

      bool goodGammaRegion1 = region > 0.5 && region < 1.5 && energy > 0.125 && e9e25 > 0.7;
      bool goodGammaRegion2 = region > 1.5 && region < 2.5 && energy > 0.100;
      bool goodGammaRegion3 = region > 2.5 && region < 3.5 && energy > 0.150;
      //bool goodTiming       = timing > 800 && timing < 2400;

      //if ((goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3) && goodTiming) {
      if (goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3) {
        m_goodNeutralECLEnergy += cluster->getEnergy();
        m_iGoodNeutralClusters++;
      }
    } else {
      m_chargedECLEnergy += cluster->getEnergy();
      m_iChargedClusters++;
    }
  }

  StoreArray<Track>  tracks;
  m_iTracks = tracks.getEntries();

  StoreArray<MCParticle> mcParticles;
  m_iMCParticles = (int) mcParticles.getEntries();

  StoreArray<Particle> particles;
  m_iParticles = (int) particles.getEntries();


}
