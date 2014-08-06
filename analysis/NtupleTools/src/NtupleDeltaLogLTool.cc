/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleDeltaLogLTool.h>
#include <analysis/VariableManager/PIDVariables.h>

#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleDeltaLogLTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_fDLogLPion = new float[nDecayProducts];
  m_fDLogLKaon = new float[nDecayProducts];
  m_fDLogLProt = new float[nDecayProducts];
  m_fDLogLElec = new float[nDecayProducts];
  m_fDLogLMuon = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_DLLPion").c_str(),  &m_fDLogLPion[iProduct], (strNames[iProduct] + "_DLLPion/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_DLLKaon").c_str(),  &m_fDLogLKaon[iProduct], (strNames[iProduct] + "_DLLKaon/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_DLLProt").c_str(),  &m_fDLogLProt[iProduct], (strNames[iProduct] + "_DLLProt/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_DLLElec").c_str(),  &m_fDLogLElec[iProduct], (strNames[iProduct] + "_DLLElec/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_DLLMuon").c_str(),  &m_fDLogLMuon[iProduct], (strNames[iProduct] + "_DLLMuon/F").c_str());
  }
}

void NtupleDeltaLogLTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleDeltaLogLTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    m_fDLogLPion[iProduct] = Variable::particleDeltaLogLPion(selparticles[iProduct]);
    m_fDLogLKaon[iProduct] = Variable::particleDeltaLogLKaon(selparticles[iProduct]);
    m_fDLogLProt[iProduct] = Variable::particleDeltaLogLProton(selparticles[iProduct]);
    m_fDLogLElec[iProduct] = Variable::particleDeltaLogLElectron(selparticles[iProduct]);
    m_fDLogLMuon[iProduct] = Variable::particleDeltaLogLMuon(selparticles[iProduct]);
  }
}
