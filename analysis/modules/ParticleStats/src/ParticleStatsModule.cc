/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: ParticleStats                                            *
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
  setDescription("Make a Summary of specific ParticleLists.");
  //Parameter definition
  addParam("strParticleLists", m_strParticleLists, "List of ParticleLists", vector<string>());
}

void ParticleStatsModule::initialize()
{

  int nParticleLists = m_strParticleLists.size();
  B2INFO("Number of ParticleLists studied " << nParticleLists << " ");

  m_PassMatrix = new TMatrix(nParticleLists, nParticleLists + 1);
  m_MultiplicityMatrix = new TMatrix(nParticleLists, 2);

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
      B2WARNING("ParticleListi " << m_strParticleLists[iList] << " not found");
      continue;
    } else {
      if (!particlelist->getListSize())continue;

      pass = true;
      (*m_MultiplicityMatrix)(iList, 0) = (*m_MultiplicityMatrix)(iList, 0) + 1;
      (*m_MultiplicityMatrix)(iList, 1) = (*m_MultiplicityMatrix)(iList, 1) + particlelist->getListSize();

      for (int jList = 0; jList < nParticleLists; ++jList) {
        StoreObjPtr<ParticleList> particlelistj(m_strParticleLists[jList]);
        if (!particlelistj) {
          B2WARNING("ParticleListj " << m_strParticleLists[jList] << " not found");
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
  int npi0 = 0, npi = 0, ngamma = 0, nk = 0, ne = 0, np = 0, nmu = 0;
  for (int i = 0; i < Particles.getEntries(); i++) {
    const Particle* part = Particles[i];
    std::cout << part->getPDGCode() << " ";
    int id = part->getPDGCode();
    if (abs(id) == 111)npi0++;
    if (abs(id) == 211)npi++;
    if (abs(id) == 22)ngamma++;
    if (abs(id) == 321)nk++;
    if (abs(id) == 11)ne++;
    if (abs(id) == 13)nmu++;
    if (abs(id) == 2212)np++;


  }
//    std::cout<<"gam "<< ngamma<<" pi0 "<<npi0<<" pi "<<npi<<" nk "<<nk<<" ne "<<ne<<" np "<<np<<" nmu "<<nmu<<"\n";

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

  std::cout << "=======================================================\n";
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

  std::cout << "\n=======================================================\n";
  std::cout << "\t\t\t";
  std::cout << "| Average Candidate Multiplicity\n";
  for (int iList = 0; iList < nParticleLists; ++iList) {
    std::cout << Form("%14s(%2d)", m_strParticleLists[iList].c_str(), iList) << "\t|";
    std::cout << "\t" << Form("%6.4f", (*m_MultiplicityMatrix)(iList, 1) / m_nEvents);
    std::cout << "\t" << Form("%6.4f", (*m_MultiplicityMatrix)(iList, 1) / m_nEvents / (*m_PassMatrix)(iList, iList));
    std::cout << "\n";
  }


  std::cout << "\n=======================================================\n";
  std::cout << "Total Retention: " << Form("%6.4f\n", (float)m_nPass / (float)m_nEvents);
  std:: cout << "Total Number of Particles created in the DataStore: " << m_nParticles;
  std::cout << "\n=======================================================\n";
}
