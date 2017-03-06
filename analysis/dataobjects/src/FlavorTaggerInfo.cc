/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca and Fernando Abudinen                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/FlavorTaggerInfo.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/logging/Logger.h>
#include <iostream>

using namespace Belle2;



/************************SETTERS***********************/


void FlavorTaggerInfo::setUseModeFlavorTagger(std::string mode)
{
  m_useModeFlavorTagger = mode;
}

void FlavorTaggerInfo::addMethodMap(std::string method)
{
  StoreArray<FlavorTaggerInfoMap> flavTagInfoMap;
  if (m_methodMap.find(method) == m_methodMap.end()) {
    // create FlavorTaggerInfoMap object

    FlavorTaggerInfoMap* flavTagMap = flavTagInfoMap.appendNew();
    m_methodMap.insert(std::pair<std::string, FlavorTaggerInfoMap*>(method, flavTagMap));

  } else {
    B2FATAL("Method  " << method << " already added. Please use another name.");
  }
}


void FlavorTaggerInfo::setTrack(Track* track)
{
  m_tracks.push_back(track);
}


void FlavorTaggerInfo::setCatProb(float catProb)
{
  m_categoryProb.push_back(catProb);
}


void FlavorTaggerInfo::setTargProb(float targProb)
{
  m_targetProb.push_back(targProb);
}

void FlavorTaggerInfo::setP(float momentum)
{
  m_P.push_back(momentum);
}

void FlavorTaggerInfo::setParticle(Particle* particle)
{
  if (particle == NULL) {
    Particle* particle_empty = new Particle();
    m_particle.push_back(particle_empty);
  } else {
    m_particle.push_back(particle);
  }
}

void FlavorTaggerInfo::setMCParticle(MCParticle* particle)
{
  if (particle == NULL) {
    MCParticle* particle_empty = new MCParticle();
    m_MCparticle.push_back(particle_empty);
  } else {
    m_MCparticle.push_back(particle);
  }
}

void FlavorTaggerInfo::setMCParticleMother(MCParticle* particle)
{
  if (particle == NULL) {
    MCParticle* particle_empty = new MCParticle();
    m_MCparticle_mother.push_back(particle_empty);
  } else {
    m_MCparticle_mother.push_back(particle);
  }
}

void FlavorTaggerInfo::setIsFromB(int isIt)
{
  m_isFromB.push_back(isIt);
}

void FlavorTaggerInfo::setGoodTracksPurityFT(float pur)
{
  m_goodTracksPurityFT = pur;
}

void FlavorTaggerInfo::setGoodTracksPurityROE(float pur)
{
  m_goodTracksPurityROE = pur;
}

void FlavorTaggerInfo::setGoodTracksROE(int goodTracks)
{
  m_goodTracksROE = goodTracks;
}

void FlavorTaggerInfo::setBadTracksROE(int badTraks)
{
  m_badTracksROE = badTraks;
}

void FlavorTaggerInfo::setGoodTracksFT(int goodTracks)
{
  m_goodTracksFT = goodTracks;
}

void FlavorTaggerInfo::setBadTracksFT(int badTraks)
{
  m_badTracksFT = badTraks;
}


void FlavorTaggerInfo::setProdPointResolutionZ(float deltaProdZ)
{
  m_prodPointResZ.push_back(deltaProdZ);
}

void FlavorTaggerInfo::setD0(double D0)
{
  m_D0.push_back(float(D0));
}

void FlavorTaggerInfo::setZ0(double Z0)
{
  m_Z0.push_back(float(Z0));
}

void FlavorTaggerInfo::setCategories(std::string category)
{
  m_categories.push_back(category);
}


/************************GETTERS***********************/


std::string FlavorTaggerInfo::getUseModeFlavorTagger()
{
  return m_useModeFlavorTagger;
}

FlavorTaggerInfoMap* FlavorTaggerInfo::getMethodMap(std::string method)
{
  auto it = m_methodMap.find(method);
  if (it == m_methodMap.end()) {
    B2FATAL("Method  " << method << " does not exist in the FlavorTaggerInfoMap. Please add it before to combinerMethods.");
  } else {
    return it -> second;
  }
}

bool FlavorTaggerInfo::isMethodInMap(std::string method)
{
  auto it = m_methodMap.find(method);
  if (it == m_methodMap.end()) {
    return false;
  } else {
    return true;
  }
}

std::vector<float> FlavorTaggerInfo::getCatProb()
{
  return m_categoryProb;
}

std::vector<float> FlavorTaggerInfo::getTargProb()
{
  return m_targetProb;
}

std::vector<float> FlavorTaggerInfo::getP()
{
  return m_P;
}

std::vector<Particle*> FlavorTaggerInfo::getParticle()
{
  return m_particle;
}

std::vector<MCParticle*> FlavorTaggerInfo::getMCParticle()
{
  return m_MCparticle;
}

std::vector<MCParticle*> FlavorTaggerInfo::getMCParticleMother()
{
  return m_MCparticle_mother;
}

std::vector<Track*> FlavorTaggerInfo::getTracks()
{
  for (unsigned i = 0; i < m_tracks.size(); i++) {
    if (m_tracks[i]) {
    }
  }
  return m_tracks;
}

std::vector<float> FlavorTaggerInfo::getD0()
{
  return m_D0;
}

std::vector<float> FlavorTaggerInfo::getZ0()
{
  return m_Z0;
}

Track* FlavorTaggerInfo::getTrack(int track)
{
  return m_tracks[track];
}

std::vector<int> FlavorTaggerInfo::getIsFromB()
{
  return m_isFromB;
}


float FlavorTaggerInfo::getGoodTracksPurityFT()
{
  return m_goodTracksPurityFT;
}


float FlavorTaggerInfo::getGoodTracksPurityROE()
{
  return m_goodTracksPurityROE;
}


int FlavorTaggerInfo::getGoodTracksROE()
{
  return m_goodTracksROE;
}

int FlavorTaggerInfo::getBadTracksROE()
{
  return m_badTracksROE;
}

int FlavorTaggerInfo::getGoodTracksFT()
{
  return m_goodTracksFT;
}

int FlavorTaggerInfo::getBadTracksFT()
{
  return m_badTracksFT;
}


std::vector<float> FlavorTaggerInfo::getProdPointResolutionZ()
{
  return m_prodPointResZ;
}


std::vector<std::string> FlavorTaggerInfo::getCategories()
{
  return m_categories;
}


