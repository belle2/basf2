/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/ParticleList.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace Belle2;

void ParticleList::initialize(int pdg, std::string name)
{
  m_pdg    = pdg;
  m_pdgbar = pdg;

  m_thisListName = name;
  m_antiListName = "";
}

void ParticleList::setParticleCollectionName(std::string name, bool forAntiParticle)
{
  m_particleStore = name;

  if (m_antiListName.empty() || forAntiParticle == false)
    return;

  StoreObjPtr<ParticleList> antiList(m_antiListName);
  if (antiList) {
    antiList->setParticleCollectionName(name, false);
  } else
    B2ERROR("ParticleList::setParticleCollectionName unable to load anti-particle List!");
}

void ParticleList::addParticle(const Particle* particle)
{
  if (particle->getArrayName() != m_particleStore) {
    B2ERROR("ParticleList::addParticle particle is from different store array, not added");
    return;
  }

  int iparticle = particle->getArrayIndex();
  if (iparticle < 0) {
    B2ERROR("ParticleList::addParticle particle is not in a store array, not added")
    return;
  }

  int pdg = particle->getPDGCode();
  Particle::EFlavorType type = particle->getFlavorType();
  addParticle((unsigned) iparticle, pdg, type, true);
}

void ParticleList::bindAntiParticleList(ParticleList& antiList, bool includingAntiList)
{
  m_pdgbar       = antiList.getPDGCode();
  m_antiListName = antiList.getParticleListName();

  if (abs(m_pdgbar) != abs(m_pdg))
    B2ERROR("ParticleList::bindAntiParticleList invalid (inconsistent) PDG codes!");

  if (includingAntiList)
    antiList.bindAntiParticleList(*this, false);
}

void ParticleList::addParticleToAntiList(unsigned iparticle, int pdg, Particle::EFlavorType type)
{
  if (m_antiListName.empty())
    return;

  StoreObjPtr<ParticleList> antiList(m_antiListName);
  if (antiList) {
    antiList->addParticle(iparticle, pdg, type, false);
  } else
    B2ERROR("ParticleList::addParticleToAntiList unable to load anti-particle List!");
}

void ParticleList::addParticle(unsigned iparticle, int pdg, Particle::EFlavorType type, bool includingAntiList)
{
  if (abs(pdg) != abs(getPDGCode())) {
    B2ERROR("ParticleList::addParticle PDG codes do not match, not added");
    return;
  }

  if (type == Particle::c_Unflavored) {
    // this is self-conjugated particle
    // add it to the self-conjugated list of this (and anti-particle list if exists)

    // check if the particle is already in this list
    if (std::find(m_scList.begin(), m_scList.end(), iparticle) == m_scList.end()) {
      m_scList.push_back(iparticle);
    } else {
      B2WARNING("ParticleList::addParticle Trying to add Particle with index=" << iparticle
                << " to the ParticleList=" << m_thisListName << " that is already included!");
      return;
    }

    // add it to the self-conjugated list
    if (includingAntiList)
      addParticleToAntiList(iparticle, pdg, type);
  } else if (type == Particle::c_Flavored) {
    unsigned antiParticle = (pdg == getPDGCode()) ? 0 : 1;

    if (antiParticle)
      addParticleToAntiList(iparticle, pdg, type);
    else {
      if (std::find(m_fsList.begin(), m_fsList.end(), iparticle) == m_fsList.end()) {
        m_fsList.push_back(iparticle);
      } else {
        B2WARNING("ParticleList::addParticle Trying to add Particle with index=" << iparticle
                  << " to the ParticleList=" << m_thisListName << "that is already included! Particle not added");
      }
    }
  } else {
    B2ERROR("ParticleList::addParticle invalid flavor type, not added");
  }
}

void ParticleList::removeParticles(const std::vector<unsigned int>& toRemove, bool removeFromAntiList)
{
  std::vector<int> newList;
  // remove Particles from flavor-specific list of this Particle List
  for (unsigned i = 0; i < m_fsList.size(); ++i) {
    if (std::find(toRemove.begin(), toRemove.end(), m_fsList[i]) == toRemove.end())
      newList.push_back(m_fsList[i]);
  }
  m_fsList = newList;

  // remove Particles from self-conjugated list of this Particle List
  newList.clear();
  for (unsigned i = 0; i < m_scList.size(); ++i) {
    if (std::find(toRemove.begin(), toRemove.end(), m_scList[i]) == toRemove.end())
      newList.push_back(m_scList[i]);
  }
  m_scList = newList;

  if (!removeFromAntiList)
    return;

  // Remove also particles form anti-particle list
  if (m_antiListName.empty())
    return;

  StoreObjPtr<ParticleList> antiList(m_antiListName);
  if (antiList)
    antiList->removeParticles(toRemove, false);
}

Particle* ParticleList::getParticle(unsigned i, bool includingAntiList) const
{
  StoreArray<Particle> Particles(m_particleStore);

  if (i < m_fsList.size()) {
    return Particles[m_fsList[i]];
  } else if (i < m_fsList.size() + m_scList.size()) {
    i -= m_fsList.size();
    return Particles[m_scList[i]];
  }

  if (!includingAntiList)
    return 0;

  if (m_antiListName.empty())
    return 0;

  StoreObjPtr<ParticleList> antiList(m_antiListName);
  if (antiList)
    return antiList->getParticle(i - m_fsList.size() - m_scList.size(), false);

  return 0;
}

unsigned ParticleList::getListSize(bool includingAntiList) const
{
  unsigned size = 0;

  // FlavorSpecific particles of this list
  size += m_fsList.size();
  // SelfConjugated particles of this list = SelfConjugated particles of anti-particle list
  size += m_scList.size();

  if (includingAntiList) {
    // FlavorSpecific particles of anti-particle list
    size += getNParticlesOfType(EParticleType::c_FlavorSpecificParticle, true);
  }

  return size;
}

std::vector<int> ParticleList::getList(EParticleType K, bool forAntiParticle) const
{
  if (!forAntiParticle) {
    if (K == c_FlavorSpecificParticle)
      return m_fsList;
    else
      return m_scList;
  } else {
    std::vector<int> emptyList;

    if (m_antiListName.empty())
      return emptyList;

    StoreObjPtr<ParticleList> antiList(m_antiListName);
    if (antiList)
      return antiList->getList(K);

    return emptyList;
  }
}

bool ParticleList::contains(const Particle* p, bool includingAntiList) const
{
  const int index = p->getArrayIndex();
  for (int i = 0; i < 3; i++) {
    if (i == 1 && !includingAntiList)
      continue;

    const std::vector<int>& currentList = getList((i < 2) ? c_FlavorSpecificParticle : c_SelfConjugatedParticle, i == 1);
    if (std::find(currentList.begin(), currentList.end(), index) != currentList.end())
      return true;
  }
  return false;
}

void ParticleList::print() const
{
  unsigned thisFSCount = getNParticlesOfType(c_FlavorSpecificParticle);
  unsigned thisSCCount = getNParticlesOfType(c_SelfConjugatedParticle);
  unsigned antiFSCount = getNParticlesOfType(c_FlavorSpecificParticle, true);
  unsigned antiSCCount = getNParticlesOfType(c_SelfConjugatedParticle, true);

  if (!m_antiListName.empty()) {
    B2INFO(" ParticleLists: " << m_thisListName << " (" << thisFSCount << "+" << thisSCCount << ")"
           << " + " << m_antiListName << " (" << antiFSCount << "+" << antiSCCount << ")");
  } else {
    B2INFO(" ParticleList : " << m_thisListName << " (" << thisFSCount << "+" << thisSCCount << ")");
  }
}

ClassImp(ParticleList)

