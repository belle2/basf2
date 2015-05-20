/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/FlavorTagInfo.h>

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;



/************************SETTERS***********************/

void FlavorTagInfo::setTrack(Belle2::Track* track)
{
  m_tracks.push_back(track);
}

void FlavorTagInfo::setTracks(std::vector<Belle2::Track*> tracks)
{

  for (unsigned i = 0; i < tracks.size(); i++) {
    m_tracks.push_back(tracks[i]);
  }
}


void FlavorTagInfo::setCatProb(float catProb)
{
  m_categoryProb.push_back(catProb);
}


void FlavorTagInfo::setTargProb(float targProb)
{
  m_targetProb.push_back(targProb);
}

void FlavorTagInfo::setP(float momentum)
{
  m_P.push_back(momentum);
}

void FlavorTagInfo::setParticle(Particle* particle)
{
  if (particle == NULL) {
    Particle* particle_empty = new Particle();
    m_particle.push_back(particle_empty);
  } else {
    m_particle.push_back(particle);
  }
}

void FlavorTagInfo::setMCParticle(MCParticle* particle)
{
  if (particle == NULL) {
    MCParticle* particle_empty = new MCParticle();
    m_MCparticle.push_back(particle_empty);
  } else {
    m_MCparticle.push_back(particle);
  }
}

void FlavorTagInfo::setMCParticleMother(MCParticle* particle)
{
  if (particle == NULL) {
    MCParticle* particle_empty = new MCParticle();
    m_MCparticle_mother.push_back(particle_empty);
  } else {
    m_MCparticle_mother.push_back(particle);
  }
}

void FlavorTagInfo::setIsFromB(int isIt)
{
  m_isFromB.push_back(isIt);
}

void FlavorTagInfo::setIsFromBGeneral(int isIt)
{
  m_isFromBGeneral = isIt;
}


void FlavorTagInfo::setROEComesFromB(int isIt)
{
  m_ROEComesFromB = isIt;
}

void FlavorTagInfo::setGoodTracksPurityFT(float pur)
{
  m_goodTracksPurityFT = pur;
}

void FlavorTagInfo::setGoodBadTracksRatioFT(float ratio)
{
  m_goodBadTracksRatioFT = ratio;
}

void FlavorTagInfo::setGoodTracksPurityROE(float pur)
{
  m_goodTracksPurityROE = pur;
}

void FlavorTagInfo::setGoodBadTracksRatioROE(float ratio)
{
  m_goodBadTracksRatioROE = ratio;
}

void FlavorTagInfo::setGoodTracksROE(int goodTracks)
{
  m_goodTracksROE = goodTracks;
}

void FlavorTagInfo::setBadTracksROE(int badTraks)
{
  m_badTracksROE = badTraks;
}

void FlavorTagInfo::setGoodTracksFT(int goodTracks)
{
  m_goodTracksFT = goodTracks;
}

void FlavorTagInfo::setBadTracksFT(int badTraks)
{
  m_badTracksFT = badTraks;
}

void FlavorTagInfo::setFTandROEGoodTracksRatio(float ratio)
{
  m_FTandROEGoodTracksRatio = ratio;
}

void FlavorTagInfo::setProdVertexMotherZ(float prodVer)
{
  m_prodVerZ.push_back(prodVer);
}

void FlavorTagInfo::setD0(double D0)
{
  m_D0.push_back(float(D0));
}

void FlavorTagInfo::setZ0(double Z0)
{
  m_Z0.push_back(float(Z0));
}

void FlavorTagInfo::setCategories(std::string category)
{
  m_categories.push_back(category);
}


/************************GETTERS***********************/


std::vector<float> FlavorTagInfo::getCatProb()
{
  return m_categoryProb;
}

std::vector<float> FlavorTagInfo::getTargProb()
{
  return m_targetProb;
}

std::vector<float> FlavorTagInfo::getP()
{
  return m_P;
}

std::vector<Particle*> FlavorTagInfo::getParticle()
{
  return m_particle;
}

std::vector<MCParticle*> FlavorTagInfo::getMCParticle()
{
  return m_MCparticle;
}

std::vector<MCParticle*> FlavorTagInfo::getMCParticleMother()
{
  return m_MCparticle_mother;
}

std::vector<Belle2::Track*> FlavorTagInfo::getTracks()
{
  for (unsigned i = 0; i < m_tracks.size(); i++) {
    if (m_tracks[i]) {
    }
  }
  return m_tracks;
}

std::vector<float> FlavorTagInfo::getD0()
{
  return m_D0;
}

std::vector<float> FlavorTagInfo::getZ0()
{
  return m_Z0;
}

Belle2::Track* FlavorTagInfo::getTrack(int track)
{
  return m_tracks[track];
}

std::vector<int> FlavorTagInfo::getIsFromB()
{
  return m_isFromB;
}

int FlavorTagInfo::getIsFromBGeneral()
{
  return m_isFromBGeneral;
}

int FlavorTagInfo::getROEComesFromB()
{
  return m_ROEComesFromB;
}

float FlavorTagInfo::getGoodTracksPurityFT()
{
  return m_goodTracksPurityFT;
}

float FlavorTagInfo::getGoodBadTracksRatioFT()
{
  return m_goodBadTracksRatioFT;
}

float FlavorTagInfo::getGoodTracksPurityROE()
{
  return m_goodTracksPurityROE;
}

float FlavorTagInfo::getGoodBadTracksRatioROE()
{
  return m_goodBadTracksRatioROE;
}

int FlavorTagInfo::getGoodTracksROE()
{
  return m_goodTracksROE;
}

int FlavorTagInfo::getBadTracksROE()
{
  return m_badTracksROE;
}

int FlavorTagInfo::getGoodTracksFT()
{
  return m_goodTracksFT;
}

int FlavorTagInfo::getBadTracksFT()
{
  return m_badTracksFT;
}

float FlavorTagInfo::getFTandROEGoodTracksRatio()
{
  return m_FTandROEGoodTracksRatio;
}


std::vector<float> FlavorTagInfo::getProdVertexMotherZ()
{
  return m_prodVerZ;
}


std::vector<std::string> FlavorTagInfo::getCategories()
{
  return m_categories;
}


/**************************/
void FlavorTagInfo::relationToParticle(Particle* particle)
{
  particle->addRelationTo(this);

}

ClassImp(FlavorTagInfo)
