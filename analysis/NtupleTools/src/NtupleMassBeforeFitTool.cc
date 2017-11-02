/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMassBeforeFitTool.h>
#include <analysis/VariableManager/Variables.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleMassBeforeFitTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_invM     = new float[nDecayProducts];
  m_invMerr  = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_InvM").c_str(),  &m_invM[iProduct], (strNames[iProduct] + "_InvM/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrM").c_str(),  &m_invMerr[iProduct], (strNames[iProduct] + "_ErrM/F").c_str());
  }
}

void NtupleMassBeforeFitTool::deallocateMemory()
{
  delete [] m_invM;
  delete [] m_invMerr;
}

void NtupleMassBeforeFitTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMassBeforeFitTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_invM[iProduct]     = Variable::particleInvariantMass(selparticles[iProduct]);
    m_invMerr[iProduct]  = Variable::particleInvariantMassError(selparticles[iProduct]);
  }
}
