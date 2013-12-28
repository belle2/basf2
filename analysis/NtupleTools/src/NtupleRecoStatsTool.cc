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
#include <tracking/dataobjects/Track.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <tracking/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

void NtupleRecoStatsTool::setupTree()
{
  m_iPhotons = -1;
  m_iPi0s = -1;
  m_iTracks = -1;
  m_iMCParticles = -1;
  m_iParticles = -1;
  m_tree->Branch("nECLGammas",  &m_iPhotons,    "nECLGammas/I");
  m_tree->Branch("nECLPi0s",  &m_iPi0s,     "nECLPi0s/I");
  m_tree->Branch("nTracks" ,  &m_iTracks,     "nTracks/I");
  m_tree->Branch("nMCParticles", &m_iMCParticles,    "nMCParticles/I");
  m_tree->Branch("nParticles" , &m_iParticles,    "nParticles/I");

}

void NtupleRecoStatsTool::eval(const  Particle*)
{
  StoreArray<ECLGamma>    eclgammas("ECLGammas");
  m_iPhotons = (int) eclgammas.getEntries();

  StoreArray<ECLPi0>   pi0s;
  m_iPi0s = (int) pi0s.getEntries();

  StoreArray<Track>  tracks("Tracks");
  m_iTracks = tracks.getEntries();

  StoreArray<MCParticle> mcParticles("");
  m_iMCParticles = (int) mcParticles.getEntries();

  StoreArray<Particle> particles;
  m_iParticles = (int) particles.getEntries();
}
