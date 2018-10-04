/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/NtupleTools/NtupleXpTool.h>
#include <analysis/variables/Variables.h>
#include <analysis/variables/EventVariables.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleXpTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_nDecayProducts = strNames.size();

  m_fxp = new float[m_nDecayProducts];

  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    m_tree-> Branch((strNames[iProduct] + "_xp").c_str(), &m_fxp[iProduct], (strNames[iProduct] + "_xp").c_str());
  }

}

void NtupleXpTool::deallocateMemory()
{
  delete [] m_fxp;
}

void NtupleXpTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleXpTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  PCmsLabTransform T;

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fxp[iProduct] = Variable::particleXp(selparticles[iProduct]);
  }
}



