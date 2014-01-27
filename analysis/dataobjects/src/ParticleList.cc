/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/dataobjects/ParticleList.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

#include <iostream>

using namespace std;
using namespace Belle2;

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
  unsigned type = particle->getFlavorType();
  addParticle((unsigned) iparticle, pdg, type);
}

void ParticleList::addParticle(unsigned iparticle, int pdg, unsigned type)
{
  if (abs(pdg) != abs(m_pdg)) {
    B2ERROR("ParticleList::addParticle PDG codes do not match, not added")
    return;
  }
  if (type != m_flavorType) {
    B2ERROR("ParticleList::addParticle flavor types do not match, not added")
    return;
  }
  unsigned k = 0;
  if (m_flavorType == 1) k = (pdg == m_pdg) ? 0 : 1;
  m_list[k].push_back(iparticle);
  m_good[k].push_back(true);
}

void ParticleList::markToRemove(unsigned i, unsigned K)
{
  if (K > m_flavorType) return;
  if (i > m_list[K].size()) return;
  m_good[K][i] = false;
}

void ParticleList::markToRemove(unsigned i)
{
  if (i < m_list[0].size()) {
    m_good[0][i] = false;
  } else {
    i -= m_list[0].size();
    if (i < m_list[1].size()) {
      m_good[1][i] = false;
    }
  }
}

void ParticleList::removeMarked()
{
  for (unsigned k = 0; k < m_flavorType + 1; k++) {
    for (int i = 0; i < (int)m_list[k].size(); ++i) {
      if (!m_good[k][i]) {
        m_list[k].erase(m_list[k].begin() + i);
        m_good[k].erase(m_good[k].begin() + i);
        --i;
      }
    }
  }
}

Particle* ParticleList::getParticle(unsigned i) const
{
  StoreArray<Particle> Particles(m_particleStore);
  if (i < m_list[0].size()) {
    return Particles[m_list[0][i]];
  } else {
    i -= m_list[0].size();
    if (i < m_list[1].size()) {
      return Particles[m_list[1][i]];
    } else {
      return 0;
    }
  }
}

void ParticleList::print() const
{
  std::cout << "ParticleList:";
  std::cout << " " << m_particleStore;
  std::cout << " PDGCode=" << m_pdg;
  std::cout << " flavorType=" << m_flavorType;
  if (m_flavorType == 0) {
    std::cout << " size=" << m_list[0].size();
  } else {
    std::cout << " size=" << m_list[0].size() << "+" << m_list[1].size();
  }
  std::cout << " arrayIndices: ";
  for (unsigned k = 0; k < m_flavorType + 1; k++) {
    for (unsigned i = 0; i < m_list[k].size(); i++) {
      std::cout << m_list[k][i] << ", ";
    }
  }
  std::cout << " flags: ";
  for (unsigned k = 0; k < m_flavorType + 1; k++) {
    for (unsigned i = 0; i < m_list[k].size(); i++) {
      std::cout << m_good[k][i];
    }
  }
  std::cout << std::endl;
}

void ParticleList::setFlavorType()
{
  m_flavorType = 1; // flavored particle
  if (m_pdg < 0) return;
  if (m_pdg == 22) {m_flavorType = 0; return;} // gamma
  if (m_pdg == 310) {m_flavorType = 0; return;} // K_s
  if (m_pdg == 130) {m_flavorType = 0; return;} // K_L
  int nnn = m_pdg / 10;
  int q3 = nnn % 10; nnn /= 10;
  int q2 = nnn % 10; nnn /= 10;
  int q1 = nnn % 10;
  if (q1 == 0 && q2 == q3) m_flavorType = 0; // unflavored meson
}


ClassImp(ParticleList)



