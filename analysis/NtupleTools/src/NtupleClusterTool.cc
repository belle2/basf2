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
#include <analysis/VariableManager/Variables.h>
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

  m_uncorrE = new float[nDecayProducts];
  m_highestE = new float[nDecayProducts];
  m_timing = new float[nDecayProducts];
  m_theta = new float[nDecayProducts];
  m_phi = new float[nDecayProducts];
  m_distance = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_clusterReg").c_str(),        &m_region[iProduct], (strNames[iProduct] + "_clusterReg/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterE9E25").c_str(),      &m_e9e25[iProduct], (strNames[iProduct] + "_clusterE9E25/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterNHits").c_str(),      &m_nHits[iProduct], (strNames[iProduct] + "_clusterNHits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTrackMatch").c_str(), &m_trackM[iProduct], (strNames[iProduct] + "_clusterTrackMatch/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterUncorrE").c_str(),    &m_uncorrE[iProduct], (strNames[iProduct] + "_clusterUncorrE/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterHighE").c_str(),      &m_highestE[iProduct], (strNames[iProduct] + "_clusterHighE/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTiming").c_str(),     &m_timing[iProduct], (strNames[iProduct] + "_clusterTiming/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTheta").c_str(),      &m_theta[iProduct], (strNames[iProduct] + "_clusterTheta/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterPhi").c_str(),        &m_phi[iProduct], (strNames[iProduct] + "_clusterPhi/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterR").c_str(),          &m_distance[iProduct], (strNames[iProduct] + "_clusterR/F").c_str());
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
    m_region[iProduct]  = int(Variable::eclClusterDetectionRegion(selparticles[iProduct]));
    m_e9e25[iProduct]   = Variable::eclClusterE9E25(selparticles[iProduct]);
    m_nHits[iProduct]   = int(Variable::eclClusterNHits(selparticles[iProduct]));
    m_trackM[iProduct]  = int(Variable::eclClusterTrackMatched(selparticles[iProduct]));

    m_uncorrE[iProduct]  = Variable::eclClusterUncorrectedE(selparticles[iProduct]);
    m_highestE[iProduct] = Variable::eclClusterHighestE(selparticles[iProduct]);
    m_timing[iProduct]   = Variable::eclClusterTiming(selparticles[iProduct]);
    m_theta[iProduct]    = Variable::eclClusterTheta(selparticles[iProduct]);
    m_phi[iProduct]      = Variable::eclClusterPhi(selparticles[iProduct]);
    m_distance[iProduct] = Variable::eclClusterR(selparticles[iProduct]);
  }
}
