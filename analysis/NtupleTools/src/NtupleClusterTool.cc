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
#include <analysis/VariableManager/ECLVariables.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleClusterTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_region = new int[nDecayProducts];
  m_e1e9   = new float[nDecayProducts];
  m_e9e21  = new float[nDecayProducts];
  m_nHits  = new int[nDecayProducts];
  m_ClusterPSD  = new int[nDecayProducts];
  m_ClusterHadronIntensity  = new float[nDecayProducts];
  m_NumberofHadronDigits = new int[nDecayProducts];
  m_trackM = new int[nDecayProducts];

  m_uncorrE = new float[nDecayProducts];
  m_highestE = new float[nDecayProducts];
  m_timing = new float[nDecayProducts];
  m_theta = new float[nDecayProducts];
  m_phi = new float[nDecayProducts];
  m_distance = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_clusterReg").c_str(),        &m_region[iProduct],
                   (strNames[iProduct] + "_clusterReg/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterE1E9").c_str(),       &m_e1e9[iProduct],
                   (strNames[iProduct] + "_clusterE1E9/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterE9E21").c_str(),      &m_e9e21[iProduct],
                   (strNames[iProduct] + "_clusterE9E21/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterNHits").c_str(),      &m_nHits[iProduct],
                   (strNames[iProduct] + "_clusterNHits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterPSD").c_str(),      &m_ClusterPSD[iProduct],
                   (strNames[iProduct] + "_clusterPSD/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterHadronIntensity").c_str(),      &m_ClusterHadronIntensity[iProduct],
                   (strNames[iProduct] + "_clusterHadronIntensity/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterNumberofHadronDigits").c_str(),      &m_NumberofHadronDigits[iProduct],
                   (strNames[iProduct] + "_clusterNumberofHadronDigits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTrackMatch").c_str(), &m_trackM[iProduct],
                   (strNames[iProduct] + "_clusterTrackMatch/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterUncorrE").c_str(),    &m_uncorrE[iProduct],
                   (strNames[iProduct] + "_clusterUncorrE/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterHighE").c_str(),      &m_highestE[iProduct],
                   (strNames[iProduct] + "_clusterHighE/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTiming").c_str(),     &m_timing[iProduct],
                   (strNames[iProduct] + "_clusterTiming/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterTheta").c_str(),      &m_theta[iProduct],
                   (strNames[iProduct] + "_clusterTheta/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterPhi").c_str(),        &m_phi[iProduct],
                   (strNames[iProduct] + "_clusterPhi/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_clusterR").c_str(),          &m_distance[iProduct],
                   (strNames[iProduct] + "_clusterR/F").c_str());
  }
}

void NtupleClusterTool::deallocateMemory()
{
  delete [] m_region;
  delete [] m_e1e9;
  delete [] m_e9e21;
  delete [] m_ClusterPSD;
  delete [] m_ClusterHadronIntensity;
  delete [] m_NumberofHadronDigits;
  delete [] m_nHits;
  delete [] m_trackM;

  delete [] m_uncorrE;
  delete [] m_highestE;
  delete [] m_timing;
  delete [] m_theta;
  delete [] m_phi;
  delete [] m_distance;
}

void NtupleClusterTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleClusterTool::eval - no Particle found!");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_region[iProduct]  = int(Variable::eclClusterDetectionRegion(selparticles[iProduct]));
    m_e1e9[iProduct]    = Variable::eclClusterE1E9(selparticles[iProduct]);
    m_e9e21[iProduct]   = Variable::eclClusterE9E21(selparticles[iProduct]);
    m_nHits[iProduct]   = int(Variable::eclClusterNHits(selparticles[iProduct]));
    m_ClusterPSD[iProduct]   = int(Variable::eclClusterPulseShapeDiscrimination(selparticles[iProduct]));
    m_trackM[iProduct]  = int(Variable::eclClusterTrackMatched(selparticles[iProduct]));

    m_uncorrE[iProduct]  = Variable::eclClusterUncorrectedE(selparticles[iProduct]);
    m_highestE[iProduct] = Variable::eclClusterHighestE(selparticles[iProduct]);
    m_timing[iProduct]   = Variable::eclClusterTiming(selparticles[iProduct]);
    m_theta[iProduct]    = Variable::eclClusterTheta(selparticles[iProduct]);
    m_phi[iProduct]      = Variable::eclClusterPhi(selparticles[iProduct]);
    m_distance[iProduct] = Variable::eclClusterR(selparticles[iProduct]);
    m_ClusterHadronIntensity[iProduct]  = Variable::eclClusterHadronIntensity(selparticles[iProduct]);
    m_NumberofHadronDigits[iProduct]  = int(Variable::eclClusterNumberofHadronDigits(selparticles[iProduct]));
  }
}
