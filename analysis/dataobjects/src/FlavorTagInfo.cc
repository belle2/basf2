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


void FlavorTagInfo::setCatProb(double catProb)
{
  m_categoryProb.push_back(catProb);
}


void FlavorTagInfo::setTargProb(double targProb)
{
  m_targetProb.push_back(targProb);
}


void FlavorTagInfo::setParticle(Particle* particle)
{
  m_particle.push_back(particle);
}

void FlavorTagInfo::setIsB(bool isIt)
{
  m_isB.push_back(isIt);
}

void FlavorTagInfo::setCategories(std::string category)
{
  m_categories.push_back(category);
}


/************************GETTERS***********************/


std::vector<double> FlavorTagInfo::getCatProb()
{
  return m_categoryProb;
}

std::vector<double> FlavorTagInfo::getTargProb()
{
  return m_targetProb;
}
std::vector<Particle*> FlavorTagInfo::getParticle()
{
  return m_particle;
}

const std::vector<Belle2::Track*> FlavorTagInfo::getTracks() const
{
  return m_tracks;
}

std::vector<bool> FlavorTagInfo::getIsB()
{
  return m_isB;
}

std::vector<std::string> FlavorTagInfo::getCategories()
{
  return m_categories;
}

ClassImp(FlavorTagInfo)
