/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleCMSKinematicsTool.h>
#include <analysis/VariableManager/Variables.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleCMSKinematicsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_fP = new float[nDecayProducts];
  m_fP4 = new float*[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Pcms").c_str(), &m_fP[iProduct], (strNames[iProduct] + "_Pcms/F").c_str());
    m_fP4[iProduct] = new float[4];
    m_tree->Branch((strNames[iProduct] + "_P4cms").c_str(), &m_fP4[iProduct][0], (strNames[iProduct] + "_P4cms[4]/F").c_str());
  }
}

void NtupleCMSKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleCMSKinematicsTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fP[iProduct]     = Variable::particleP_CMS(selparticles[iProduct]);
    m_fP4[iProduct][0] = Variable::particlePx_CMS(selparticles[iProduct]);
    m_fP4[iProduct][1] = Variable::particlePy_CMS(selparticles[iProduct]);
    m_fP4[iProduct][2] = Variable::particlePz_CMS(selparticles[iProduct]);
    m_fP4[iProduct][3] = Variable::particleE_CMS(selparticles[iProduct]);
  }
}
