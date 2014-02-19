/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleShowerTool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <TBranch.h>

void NtupleShowerTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_region = new int[nDecayProducts];
  m_e9e25  = new float[nDecayProducts];
  m_nHits  = new int[nDecayProducts];
  m_trackM = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_showerReg").c_str(),        &m_region[iProduct], (strNames[iProduct] + "_showerReg/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_showerE9E25").c_str(),      &m_e9e25[iProduct], (strNames[iProduct] + "_showerE9E25/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_showerNHits").c_str(),      &m_nHits[iProduct], (strNames[iProduct] + "_showerNHits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_showerTrackMatch").c_str(), &m_trackM[iProduct], (strNames[iProduct] + "_showerTrackMatch/I").c_str());
  }
}

void NtupleShowerTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleShowerTool::eval - ERROR, no Particle found!\n");
    return;
  }
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_region[iProduct]  = int(analysis::eclShowerDetectionRegion(selparticles[iProduct]));
    m_e9e25[iProduct]   = analysis::eclShowerE9E25(selparticles[iProduct]);
    m_nHits[iProduct]   = int(analysis::eclShowerNHits(selparticles[iProduct]));
    m_trackM[iProduct]  = int(analysis::eclShowerTrackMatched(selparticles[iProduct]));
  }
}
