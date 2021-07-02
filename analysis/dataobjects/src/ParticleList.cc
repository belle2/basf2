/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/dataobjects/ParticleList.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/utilities/HTML.h>

#include <iostream>
#include <algorithm>

using namespace std;
using namespace Belle2;

ParticleList::~ParticleList()
{
  delete m_antiList;
}

void ParticleList::initialize(int pdg, const std::string& name, const std::string& particleStoreName)
{
  m_pdg    = pdg;
  m_pdgbar = pdg;
  m_particleStore = particleStoreName;

  m_thisListName = name;
  m_antiListName.clear();
}

void ParticleList::setParticleCollectionName(const std::string& name, bool forAntiParticle)
{
  m_particleStore = name;

  if (forAntiParticle and !m_antiListName.empty())
    getAntiParticleList().setParticleCollectionName(name, false);
}

void ParticleList::addParticle(const Particle* particle)
{
  if (particle->getArrayName() != m_particleStore) {
    B2ERROR("ParticleList::addParticle particle is from different store array, not added");
    return;
  }

  int iparticle = particle->getArrayIndex();
  if (iparticle < 0) {
    B2ERROR("ParticleList::addParticle particle is not in a store array, not added");
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
    if (includingAntiList and !m_antiListName.empty())
      getAntiParticleList().addParticle(iparticle, pdg, type, false);
  } else if (type == Particle::c_Flavored) {
    unsigned antiParticle = (pdg == getPDGCode()) ? 0 : 1;

    if (antiParticle)
      getAntiParticleList().addParticle(iparticle, pdg, type, false);
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
  for (int i : m_fsList) {
    if (std::find(toRemove.begin(), toRemove.end(), i) == toRemove.end())
      newList.push_back(i);
  }
  m_fsList = newList;

  // remove Particles from self-conjugated list of this Particle List
  newList.clear();
  for (int i : m_scList) {
    if (std::find(toRemove.begin(), toRemove.end(), i) == toRemove.end())
      newList.push_back(i);
  }
  m_scList = newList;

  if (removeFromAntiList and !m_antiListName.empty()) {
    getAntiParticleList().removeParticles(toRemove, false);
  }
}

void ParticleList::clear(bool includingAntiList)
{
  m_fsList.clear();
  m_scList.clear();

  if (includingAntiList and !m_antiListName.empty()) {
    getAntiParticleList().clear(false);
  }
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

  if (includingAntiList and !m_antiListName.empty())
    return getAntiParticleList().getParticle(i - m_fsList.size() - m_scList.size(), false);

  return nullptr;
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

const std::vector<int>& ParticleList::getList(EParticleType K, bool forAntiParticle) const
{
  if (!forAntiParticle) {
    if (K == c_FlavorSpecificParticle)
      return m_fsList;
    else
      return m_scList;
  } else {
    const static std::vector<int> emptyList;
    if (m_antiListName.empty())
      return emptyList;

    return getAntiParticleList().getList(K);
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
  B2INFO(HTML::htmlToPlainText(getInfoHTML()));
}

std::string ParticleList::getInfoHTML() const
{
  std::stringstream stream;
  unsigned thisFSCount = getNParticlesOfType(c_FlavorSpecificParticle);
  unsigned thisSCCount = getNParticlesOfType(c_SelfConjugatedParticle);
  unsigned antiFSCount = getNParticlesOfType(c_FlavorSpecificParticle, true);
  unsigned antiSCCount = getNParticlesOfType(c_SelfConjugatedParticle, true);

  if (!m_antiListName.empty()) {
    stream << " ParticleLists: " << m_thisListName << " (" << thisFSCount << "+" << thisSCCount << ")"
           << " + " << m_antiListName << " (" << antiFSCount << "+" << antiSCCount << ")";
  } else {
    stream << " ParticleList : " << m_thisListName << " (" << thisFSCount << "+" << thisSCCount << ")";
  }
  return HTML::escape(stream.str());
}

ParticleList& ParticleList::getAntiParticleList() const
{
  if (!m_antiList) {
    m_antiList = new StoreObjPtr<ParticleList>(m_antiListName);
    if (!m_antiList->isValid()) {
      B2FATAL("Anti-particle list " << m_antiListName << " for " << m_thisListName <<
              " not found, even though one was set via bindAntiParticleList(). Maybe you only saved one list into a .root file?");
    }
  }
  return **m_antiList;
}

