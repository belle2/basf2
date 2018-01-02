/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleROEGammasTool.h>
#include <TBranch.h>

#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/VariableManager/Variables.h>

using namespace Belle2;
using namespace std;

void NtupleROEGammasTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_nROEGoodGammas = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nROEGoodGammas").c_str(), &m_nROEGoodGammas[iProduct],
                   (strNames[iProduct] + "_nROEGoodGammas/I").c_str());
    if (nDecayProducts == 1) {
      m_tree->Branch((strNames[iProduct] + "_ROEGoodGamma_P").c_str(), &m_fP[0], (strNames[iProduct] + "_P[100]/F").c_str());
      m_tree->Branch((strNames[iProduct] + "_ROEGoodGamma_P4").c_str(), &m_fP4[0], (strNames[iProduct] + "_P4[100][4]/F").c_str());
    }
  }
}

void NtupleROEGammasTool::eval(const Particle* particle)
{
  for (int i = 0; i < 100; i++) {
    m_fP[i] = -100;
    for (int j = 0; j < 4; j++) {
      m_fP4[i][j] = -100;
    }
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_nROEGoodGammas[iProduct]  = -1;

    const RestOfEvent* roe = selparticles[iProduct]->getRelatedTo<RestOfEvent>();

    if (roe) {
      const auto& remainECLClusters = roe->getECLClusters();
      int result = 0;
      for (auto& remainECLCluster : remainECLClusters) {
        Particle gamma(remainECLCluster);
        if (Variable::goodGamma(&gamma) > 0) {
          m_fP[result] = gamma.getP();
          m_fP4[result][0] = gamma.getPx();
          m_fP4[result][1] = gamma.getPy();
          m_fP4[result][2] = gamma.getPz();
          m_fP4[result][3] = gamma.getEnergy();

          result++;
          if (result > 99) B2ERROR("Increase the number of ROE photons");
        }
      }

      m_nROEGoodGammas[iProduct]  = result;

    }
  }
}
