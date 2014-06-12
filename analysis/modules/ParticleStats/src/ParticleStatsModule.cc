/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: ParticleStats, Anze Zupanc                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/modules/ParticleStats/ParticleStatsModule.h>
#include <analysis/dataobjects/ParticleList.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cmath>
#include <algorithm>
#include <TParameter.h>
using namespace std;
using namespace Belle2;
using namespace boost::algorithm;

// Register module in the framework
REG_MODULE(ParticleStats)

ParticleStatsModule::ParticleStatsModule() : Module()
{
  //Set module properties
  setDescription("Make a summary of specific ParticleLists.");
  //Parameter definition
  addParam("particleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
}

void ParticleStatsModule::initialize()
{
  unsigned nParticleLists = m_strParticleLists.size();
  for (unsigned i = 0; i < nParticleLists; ++i) {
    bool valid = m_decaydescriptor.init(m_strParticleLists[i]);
    if (!valid)
      B2ERROR("Invalid input list name: " << m_strParticleLists[i]);

    int nProducts = m_decaydescriptor.getNDaughters();
    if (nProducts > 0)
      B2ERROR("ParticleStatsModule::initialize Invalid input DecayString " << m_strParticleLists[i]
              << ". DecayString should not contain any daughters, only the mother particle.");
  }

  B2INFO("Number of ParticleLists studied " << nParticleLists << " ");

  m_PassMatrix = new TMatrix(nParticleLists, nParticleLists + 1);
  m_MultiplicityMatrix = new TMatrix(nParticleLists, 4); // 0 All particles; 1 Negative; 2 Positive; 3 SelfConjugated

  m_nEvents = 0;

  m_nPass = 0;

  m_nParticles = 0;
}

void ParticleStatsModule::event()
{
  // number of ParticleLists
  int nParticleLists = m_strParticleLists.size();
  bool unique = true;
  bool pass = false;
  for (int iList = 0; iList < nParticleLists; ++iList) {

    StoreObjPtr<ParticleList> particlelist(m_strParticleLists[iList]);
    if (!particlelist) {
      B2INFO("ParticleListi " << m_strParticleLists[iList] << " not found");
      continue;
    } else {
      if (!particlelist->getListSize())continue;

      pass = true;
      // All Particles&Anti-Particles
      (*m_MultiplicityMatrix)(iList, 0) = (*m_MultiplicityMatrix)(iList, 0)
                                          + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle)
                                          + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true)
                                          + particlelist->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle);

      // Particles
      if (particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle)) {
        (*m_MultiplicityMatrix)(iList, 1) = (*m_MultiplicityMatrix)(iList, 1)
                                            + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle);
      }

      // Anti-Particles
      if (particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true)) {
        (*m_MultiplicityMatrix)(iList, 2) = (*m_MultiplicityMatrix)(iList, 2)
                                            + particlelist->getNParticlesOfType(ParticleList::c_FlavorSpecificParticle, true);
      }

      if (particlelist->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle)) {
        (*m_MultiplicityMatrix)(iList, 3) = (*m_MultiplicityMatrix)(iList, 3)
                                            + particlelist->getNParticlesOfType(ParticleList::c_SelfConjugatedParticle);
      }


      for (int jList = 0; jList < nParticleLists; ++jList) {
        StoreObjPtr<ParticleList> particlelistj(m_strParticleLists[jList]);
        if (!particlelistj) {
          B2INFO("ParticleListj " << m_strParticleLists[jList] << " not found");
          continue;
        } else {
          if (!particlelistj->getListSize())continue;
          (*m_PassMatrix)(iList, jList) = (*m_PassMatrix)(iList, jList) + 1.;
          if (iList != jList)unique = false;

        }
      }
    }


    if (unique)(*m_PassMatrix)(iList, nParticleLists) = (*m_PassMatrix)(iList, nParticleLists) + 1.;

  }
  StoreArray<Particle> Particles;
  m_nParticles += Particles.getEntries();

  m_nEvents++;
  if (pass)m_nPass++;
}

void ParticleStatsModule::terminate()
{
  B2INFO("ParticleStats Summary: \n");

  int nParticleLists = m_strParticleLists.size();
  for (int iList = 0; iList < nParticleLists; ++iList) {
    for (int jList = 0; jList < nParticleLists + 1; ++jList) {
      (*m_PassMatrix)(iList, jList) = (*m_PassMatrix)(iList, jList) / (float) m_nEvents;
    }
  }

  for (int iList = 0; iList < nParticleLists; ++iList) {
    for (int jList = 0; jList < nParticleLists + 1; ++jList) {
      if (iList != jList && (*m_PassMatrix)(iList, iList) > 0.)(*m_PassMatrix)(iList, jList) = (*m_PassMatrix)(iList, jList) / (*m_PassMatrix)(iList, iList) ;
    }
  }

  std::cout << "=======================================================================\n";
  std::cout << "\t\t\t";
  std::cout << "|Retention";
  for (int iList = 0; iList < nParticleLists; ++iList) {
    std::cout << "|\t" << Form("%5d", iList);
  }
  std::cout << "| Unique \n";
  for (int iList = 0; iList < nParticleLists; ++iList) {
    std::cout << Form("%14s(%2d)", m_strParticleLists[iList].c_str(), iList) << "\t|";
    std::cout << "\t" << Form("%6.4f", (*m_PassMatrix)(iList, iList));
    for (int jList = 0; jList < nParticleLists + 1; ++jList) {
      if (iList != jList)std::cout << "\t" << Form("%6.4f", (*m_PassMatrix)(iList, jList));
      if (iList == jList)std::cout << "\t" << Form("%6.4f", 1.0);
    }
    std::cout << "\n";
  }

  std::cout << "\n======================================================================\n";
  std::cout << " Average Candidate Multiplicity (ACM) and ACM for Passed Events (ACMPE) \n";
  std::cout << "\t\t\t| All Particles \t\t| Particles     \t\t| AntiParticles \t\t\n";
  std::cout << "\t\t\t| ACM\t\t| ACMPE\t\t| ACM\t\t| ACMPE\t\t| ACM\t\t| ACMPE \n";
  for (int iList = 0; iList < nParticleLists; ++iList) {
    std::cout << Form("%14s(%2d)", m_strParticleLists[iList].c_str(), iList) << "\t|";

    for (int iFlav = 0; iFlav < 4; ++iFlav) {
      std::cout << "\t" << Form("%8.4f", (*m_MultiplicityMatrix)(iList, iFlav) / m_nEvents);
      std::cout << "\t" << Form("%8.4f", (*m_MultiplicityMatrix)(iList, iFlav) / m_nEvents / (*m_PassMatrix)(iList, iList));
    }
    std::cout << "\n";
  }


  std::cout << "\n=======================================================\n";
  std::cout << "Total Retention: " << Form("%6.4f\n", (float)m_nPass / (float)m_nEvents);
  std:: cout << "Total Number of Particles created in the DataStore: " << m_nParticles;
  std::cout << "\n=======================================================\n";
}
