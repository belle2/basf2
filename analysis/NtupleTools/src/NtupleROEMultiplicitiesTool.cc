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

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nROETracks").c_str(),  &m_nTracks[iProduct], (strNames[iProduct] + "_nROETracks/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nROEECLClusters").c_str(), &m_nECLClusters[iProduct], (strNames[iProduct] + "_nROEECLClusters/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nGoodROEClusters").c_str(), &m_nGoodECLClusters[iProduct], (strNames[iProduct] + "_nGoodROEClusters/I").c_str());
  }
}

void NtupleROEMultiplicitiesTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_nTracks[iProduct]     = -1;
    m_nECLClusters[iProduct] = -1;

    const RestOfEvent* roe = selparticles[iProduct]->getRelatedTo<RestOfEvent>();

    if (roe) {
      m_nTracks[iProduct]      = roe->getNTracks();
      m_nECLClusters[iProduct]  = roe->getNECLClusters();

      const std::vector<ECLCluster*> remainECLClusters = roe->getECLClusters();
      int result = 0;
      for (unsigned i = 0; i < remainECLClusters.size(); i++) {
        Particle gamma(remainECLClusters[i]);
        if (Variable::goodGamma(&gamma) > 0)
          result++;
      }
      m_nGoodECLClusters[iProduct]  = result;
    }
  }
}
