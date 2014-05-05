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

#include <TDatabasePDG.h>

#include <iostream>
#include <algorithm>

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

void ParticleList::setPDG(int pdg)
{
  auto* particlePDG = TDatabasePDG::Instance()->GetParticle(pdg);
  if (not particlePDG) {
    B2WARNING("ParticleList::setPDG Couldn't find given PDG Code in TDatabasePDG: " << pdg)
    return;
  }

  m_pdg = particlePDG->PdgCode();
  m_pdgbar = hasAntiParticle(m_pdg) ? -m_pdg : m_pdg;

}

bool ParticleList::hasAntiParticle(int pdg)
{

  if (pdg < 0) return true;
  if (pdg == 22) return false;
  if (pdg == 310) return false;
  if (pdg == 130) return false;
  int nnn = pdg / 10;
  int q3 = nnn % 10; nnn /= 10;
  int q2 = nnn % 10; nnn /= 10;
  int q1 = nnn % 10;
  if (q1 == 0 && q2 == q3) return false;

  return true;
}

void ParticleList::addParticle(unsigned iparticle, int pdg, unsigned type)
{
  if (abs(pdg) != abs(getPDG())) {
    B2ERROR("ParticleList::addParticle PDG codes do not match, not added")
    return;
  }

  unsigned K = (pdg == getPDG()) ? 0 : 1;
  if (type == 0) K = 2;
  m_list[K].push_back(iparticle);
}

void ParticleList::removeParticles(const std::vector<unsigned int>& toRemove)
{
  for (unsigned k = 0; k < 2; k++) {
    std::vector<int> newList;
    for (int i = 0; i < (int)m_list[k].size(); ++i) {
      if (std::find(toRemove.begin(), toRemove.end(), i) == toRemove.end()) {
        newList.push_back(m_list[k][i]);
      }
    }
    m_list[k] = newList;
  }
}

Particle* ParticleList::getParticle(unsigned i) const
{
  StoreArray<Particle> Particles(m_particleStore);
  if (i < m_list[0].size()) {
    return Particles[m_list[0][i]];
  } else if (i < m_list[0].size() + m_list[1].size()) {
    i -= m_list[0].size();
    return Particles[m_list[1][i]];
  } else {
    i -= m_list[0].size() + m_list[1].size();
    if (i < m_list[2].size()) {
      return Particles[m_list[2][i]];
    } else {
      return 0;
    }
  }
}

void ParticleList::print() const
{
  std::cout << "ParticleList:";
  std::cout << " " << m_particleStore;
  std::cout << " PDGCode=" << getPDG();
  std::cout << " size=" << m_list[0].size() << "+" << m_list[1].size() << "+" <<  m_list[2].size();
  std::cout << " arrayIndices: ";
  for (unsigned k = 0; k < 2; k++) {
    for (unsigned i = 0; i < m_list[k].size(); i++) {
      std::cout << m_list[k][i] << ", ";
    }
  }
  std::cout << std::endl;
}

ClassImp(ParticleList)

