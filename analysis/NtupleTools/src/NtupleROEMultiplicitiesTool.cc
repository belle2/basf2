/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>

#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/ECLVariables.h>

#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleROEMultiplicitiesTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_nTracks     = new int[nDecayProducts];
  m_nECLClusters = new int[nDecayProducts];
  m_nGoodECLClusters = new int[nDecayProducts];
  m_nKLMClusters = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nROETracks").c_str(),  &m_nTracks[iProduct], (strNames[iProduct] + "_nROETracks/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nROEECLClusters").c_str(), &m_nECLClusters[iProduct],
                   (strNames[iProduct] + "_nROEECLClusters/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nGoodROEClusters").c_str(), &m_nGoodECLClusters[iProduct],
                   (strNames[iProduct] + "_nGoodROEClusters/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nROEKLMClusters").c_str(), &m_nKLMClusters[iProduct],
                   (strNames[iProduct] + "_nROEKLMClusters/I").c_str());
  }
}

void NtupleROEMultiplicitiesTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_nTracks[iProduct]     = -1;
    m_nECLClusters[iProduct] = -1;
    m_nKLMClusters[iProduct] = -1;

    const RestOfEvent* roe = selparticles[iProduct]->getRelatedTo<RestOfEvent>();

    if (roe) {
      m_nTracks[iProduct]      = roe->getNTracks();
      m_nECLClusters[iProduct]  = roe->getNECLClusters();
      m_nKLMClusters[iProduct]  = roe->getNKLMClusters();

      const auto& remainECLClusters = roe->getECLClusters();
      int result = 0;
      for (auto& remainECLCluster : remainECLClusters) {
        Particle gamma(remainECLCluster);
        if (Variable::goodGamma(&gamma) > 0)
          result++;
      }
      m_nGoodECLClusters[iProduct]  = result;
    }
  }
}
