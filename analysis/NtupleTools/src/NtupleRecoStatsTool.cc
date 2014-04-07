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
  m_iTracks = -1;
  m_iMCParticles = -1;
  m_iParticles = -1;
  m_tree->Branch("nECLClusters",  &m_iClusters,     "nECLClusters/I");
  m_tree->Branch("nTracks" ,     &m_iTracks,      "nTracks/I");
  m_tree->Branch("nMCParticles", &m_iMCParticles, "nMCParticles/I");
  m_tree->Branch("nParticles" ,  &m_iParticles,   "nParticles/I");

}

void NtupleRecoStatsTool::eval(const  Particle*)
{
  StoreArray<ECLCluster>    eclshowers;
  m_iClusters = (int) eclshowers.getEntries();

  StoreArray<Track>  tracks;
  m_iTracks = tracks.getEntries();

  StoreArray<MCParticle> mcParticles;
  m_iMCParticles = (int) mcParticles.getEntries();

  StoreArray<Particle> particles;
  m_iParticles = (int) particles.getEntries();
}
