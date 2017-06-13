/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleKlIdTool.h>
#include <analysis/VariableManager/Variables.h>
#include <framework/gearbox/Const.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleKlIdTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_KlId = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_KlId").c_str(),  &m_KlId[iProduct], (strNames[iProduct] + "_KlId/F").c_str());
  }
}

void NtupleKlIdTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleKlIdTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    m_KlId[iProduct] = Variable::particleKlId(selparticles[iProduct]);

  }
}
