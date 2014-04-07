/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleTrackTool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleTrackTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_fdr = new float[nDecayProducts];
  m_fdz = new float[nDecayProducts];
  m_fTrPval = new float[nDecayProducts];
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_dr").c_str(), &m_fdr[iProduct], (strNames[iProduct] + "_dr/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_dz").c_str(), &m_fdz[iProduct], (strNames[iProduct] + "_dz/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TrPval").c_str(), &m_fTrPval[iProduct], (strNames[iProduct] + "_TrPval/F").c_str());
  }
}

void NtupleTrackTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleTrackTool::eval - ERROR, no Particle found!\n");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fdr[iProduct] = analysis::particleDRho(selparticles[iProduct]);
    m_fdz[iProduct] = analysis::particleDZ(selparticles[iProduct]);
    m_fTrPval[iProduct] = selparticles[iProduct]->getPValue();
  }
}
