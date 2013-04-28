/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/NtupleMaker/NtupleRecoStatsTool.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>
#include <tracking/dataobjects/Track.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <generators/dataobjects/MCParticle.h>

void NtupleRecoStatsTool::setupTree()
{
  m_iNPhotons = -1;
  m_iNShowers = -1;
  m_iNPi0s = -1;
  m_iNTracks = -1;
  m_iNMCParticles = -1;
  m_iNParticles = -1;
  m_tree->Branch("nECLGammas", &m_iNPhotons, "nECLGammas/I");
  m_tree->Branch("nECLShowers", &m_iNShowers, "nECLShowers/I");
  m_tree->Branch("nECLPi0s", &m_iNPi0s, "nECLPi0s/I");
  m_tree->Branch("nTracks" , &m_iNTracks,  "nTracks/I");
  m_tree->Branch("nMCParticles" , &m_iNMCParticles,  "nMCParticles/I");
  m_tree->Branch("nParticles" , &m_iNParticles,  "nParticles/I");

}

void NtupleRecoStatsTool::eval(const  Particle*)
{
  StoreArray<ECLGamma>    eclgammas("ECLGammas");
  m_iNPhotons = (int) eclgammas.getEntries();

  StoreArray<ECLShower>   showers;
  m_iNShowers = (int) showers.getEntries();

  StoreArray<ECLPi0>   pi0s;
  m_iNPi0s = (int) pi0s.getEntries();

  StoreArray<Track>  tracks("Tracks");
  m_iNTracks = tracks.getEntries();

  StoreArray<MCParticle> mcParticles("");
  m_iNMCParticles = (int) mcParticles.getEntries();

  StoreArray<Particle> particles;
  m_iNParticles = (int) particles.getEntries();
}
