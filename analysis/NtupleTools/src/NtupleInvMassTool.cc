/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleInvMassTool.h>
#include <analysis/VariableManager/Variables.h>
#include <framework/logging/Logger.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleInvMassTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  if (!m_strOption.empty()) {
    B2INFO("Option is: " << m_strOption);
    if (m_strOption == "BeforeFit")
      m_useMassBeforeFit = true;
    else
      B2WARNING("Invalid option used for InvMass ntuple tool: " << m_strOption);
  }

  m_invM     = new float[nDecayProducts];
  m_invMerr  = new float[nDecayProducts];
  m_invMsig  = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_M").c_str(),  &m_invM[iProduct], (strNames[iProduct] + "_M/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrM").c_str(),  &m_invMerr[iProduct], (strNames[iProduct] + "_ErrM/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_SigM").c_str(),  &m_invMsig[iProduct], (strNames[iProduct] + "_SigM/F").c_str());
  }
}

void NtupleInvMassTool::deallocateMemory()
{
  delete [] m_invM;
  delete [] m_invMerr;
  delete [] m_invMsig;
}

void NtupleInvMassTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleInvMassTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    if (m_useMassBeforeFit) {
      m_invM[iProduct]     = Variable::particleInvariantMass(selparticles[iProduct]);
    } else {
      m_invM[iProduct]     = selparticles[iProduct]->getMass();
    }

    m_invMerr[iProduct]  = Variable::particleInvariantMassError(selparticles[iProduct]);
    m_invMsig[iProduct]  = (m_invM[iProduct] - selparticles[iProduct]->getPDGMass()) / m_invMerr[iProduct];
  }
}
