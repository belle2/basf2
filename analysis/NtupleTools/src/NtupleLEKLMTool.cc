/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleLEKLMTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/L1EmulatorVariables.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLEKLMTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_nKLMClusters = 0;
  m_tree->Branch("nKLMClusters", &m_nKLMClusters, "nKLMClusters/I");

}

void NtupleLEKLMTool::eval(const Particle* particle)
{
// if (!particle) {
//   B2ERROR("NtupleDeltaEMbcTool::eval - no Particle found!");
//    return;
//  }
//vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
//  if (selparticles.empty()){cout<<"noParticle"<<endl;selparticles.push_back(NULL);// return;
//}
//const Particle* selparticles = NULL;
// Particle* selparticles = NULL;
  m_nKLMClusters = Variable::nKLMClustersLE(particle);

}
