/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleLEKLMTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLEKLMTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_nKLMClusters = 0;
  m_maxAngTM = -1.;
  m_Layer1 = -1;
  m_Layer2 = -1;

  m_tree->Branch("nKLMClusters", &m_nKLMClusters, "nKLMClusters/I");
  m_tree->Branch("Layer1KLMClusters", &m_Layer1, "Layer1KLMClusters/I");
  m_tree->Branch("Layer2KLMClusters", &m_Layer2, "Layer2KLMClusters/I");
  m_tree->Branch("maxAngtm", &m_maxAngTM, "maxAngtm/F");
  m_tree->Branch("maxAngmm", &m_maxAngMM, "maxAngmm/F");

}

void NtupleLEKLMTool::eval(const Particle* particle)
{
  m_nKLMClusters = Variable::nKLMClustersLE(particle);
  m_maxAngTM = Variable::maxAngleTMLE(particle);
  m_maxAngMM = Variable::maxAngleMMLE(particle);
  m_Layer1 = Variable::LayerKLMCluster1LE(particle);
  m_Layer2 = Variable::LayerKLMCluster2LE(particle);
}
