/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/FlavorTagInfo.h>

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;



/************************SETTERS***********************/


void FlavorTagInfo::setTargetTrackLevel(std::string category, Track* track)
{
  m_targetTrackLevel.insert(std::pair<std::string, Track*>(category, track));
}


void FlavorTagInfo::setProbTrackLevel(std::string category, float probability)
{
  m_probTrackLevel.insert(std::pair<std::string, float>(category, probability));
}


void FlavorTagInfo::setTargetEventLevel(std::string category, Belle2::Track* track)
{
  m_targetEventLevel.insert(std::pair<std::string, Track*>(category, track));
}


void FlavorTagInfo::setProbEventLevel(std::string category, float probability)
{
  m_probEventLevel.insert(std::pair<std::string, float>(category, probability));
}


void FlavorTagInfo::setQrCategory(std::string category, float qr)
{
  m_qrCategory.insert(std::pair<std::string, float>(category, qr));
}


void FlavorTagInfo::setUseModeFlavorTagger(std::string mode)
{
  m_useModeFlavorTagger = mode;
}

void FlavorTagInfo::setMethod(std::string method)
{
  m_method = method;
}

void FlavorTagInfo::setQrCombined(float qr)
{
  m_qrCombined = qr;
}


void FlavorTagInfo::setB0Probability(float B0Probability)
{
  m_B0Probability = B0Probability;
}


void FlavorTagInfo::setB0barProbability(float B0barProbability)
{
  m_B0barProbability = B0barProbability;
}


void FlavorTagInfo::setTrack(Track* track)
{
  m_tracks.push_back(track);
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

void FlavorTagInfo::setGoodTracksPurityFT(float pur)
{
  m_goodTracksPurityFT = pur;
}

void FlavorTagInfo::setGoodTracksPurityROE(float pur)
{
  m_goodTracksPurityROE = pur;
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


void FlavorTagInfo::setProdPointResolutionZ(float deltaProdZ)
{
  m_prodPointResZ.push_back(deltaProdZ);
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


std::map<std::string, Belle2::Track*> FlavorTagInfo::getTargetTrackLevel()
{
  return m_targetTrackLevel;
}

std::map<std::string, float> FlavorTagInfo::getProbTrackLevel()
{
  return m_probTrackLevel;
}

std::map<std::string, Belle2::Track*> FlavorTagInfo::getTargetEventLevel()
{
  return m_targetEventLevel;
}

std::map<std::string, float> FlavorTagInfo::getProbEventLevel()
{
  return m_probEventLevel;
}

std::map<std::string, float> FlavorTagInfo::getQrCategory()
{
  return m_qrCategory;
}

std::string FlavorTagInfo::getUseModeFlavorTagger()
{
  return m_useModeFlavorTagger;
}

std::string FlavorTagInfo::getMethod()
{
  return m_method;
}

float FlavorTagInfo::getQrCombined()
{
  return m_qrCombined;
}

float FlavorTagInfo::getB0Probability()
{
  return m_B0Probability;
}

float FlavorTagInfo::getB0barProbability()
{
  return m_B0barProbability;
}


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

std::vector<Track*> FlavorTagInfo::getTracks()
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

Track* FlavorTagInfo::getTrack(int track)
{
  return m_tracks[track];
}

std::vector<int> FlavorTagInfo::getIsFromB()
{
  return m_isFromB;
}


float FlavorTagInfo::getGoodTracksPurityFT()
{
  return m_goodTracksPurityFT;
}


float FlavorTagInfo::getGoodTracksPurityROE()
{
  return m_goodTracksPurityROE;
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


std::vector<float> FlavorTagInfo::getProdPointResolutionZ()
{
  return m_prodPointResZ;
}


std::vector<std::string> FlavorTagInfo::getCategories()
{
  return m_categories;
}


