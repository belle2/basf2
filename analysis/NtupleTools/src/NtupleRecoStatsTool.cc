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
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <analysis/VariableManager/ECLVariables.h>

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

    // Only use one hypothesis ID for ECLClusters
    if (cluster->getHypothesisId() != 5) continue;

    if (cluster->isNeutral()) {
      Particle* gamma = new Particle(cluster);

      m_neutralECLEnergy += gamma->getEnergy();
      m_iNeutralClusters++;

      if (Variable::goodGamma(gamma)) {
        m_goodNeutralECLEnergy += gamma->getEnergy();
        m_iGoodNeutralClusters++;
      }
      delete gamma;
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
