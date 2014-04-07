/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleClusterTool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleClusterTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_region = new int[nDecayProducts];
  m_e9e25  = new float[nDecayProducts];
  m_nHits  = new int[nDecayProducts];
  m_trackM = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_clusterReg").c_str(),        &m_region[iProduct], (strNames[iProduct] + "_clusterReg/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterE9E25").c_str(),      &m_e9e25[iProduct], (strNames[iProduct] + "_clusterE9E25/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterNHits").c_str(),      &m_nHits[iProduct], (strNames[iProduct] + "_clusterNHits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTrackMatch").c_str(), &m_trackM[iProduct], (strNames[iProduct] + "_clusterTrackMatch/I").c_str());
  }
}

void NtupleClusterTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleClusterTool::eval - ERROR, no Particle found!\n");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_region[iProduct]  = int(analysis::eclClusterDetectionRegion(selparticles[iProduct]));
    m_e9e25[iProduct]   = analysis::eclClusterE9E25(selparticles[iProduct]);
    m_nHits[iProduct]   = int(analysis::eclClusterNHits(selparticles[iProduct]));
    m_trackM[iProduct]  = int(analysis::eclClusterTrackMatched(selparticles[iProduct]));
  }
}
