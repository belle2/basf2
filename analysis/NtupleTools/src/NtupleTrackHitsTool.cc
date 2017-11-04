/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleTrackHitsTool.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleTrackHitsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_iNCDCHits = new int[nDecayProducts];
  m_iNSVDHits = new int[nDecayProducts];
  m_iNPXDHits = new int[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_nCDCHits").c_str(), &m_iNCDCHits[iProduct], (strNames[iProduct] + "_nCDCHits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nSVDHits").c_str(), &m_iNSVDHits[iProduct], (strNames[iProduct] + "_nSVDHits/I").c_str());
    m_tree->Branch((strNames[iProduct] + "_nPXDHits").c_str(), &m_iNPXDHits[iProduct], (strNames[iProduct] + "_nPXDHits/I").c_str());
  }
}

void NtupleTrackHitsTool::deallocateMemory()
{
  delete [] m_iNCDCHits;
  delete [] m_iNSVDHits;
  delete [] m_iNPXDHits;
}

void NtupleTrackHitsTool::eval(const Particle* particle)
{

  if (!particle) {
    B2ERROR("NtupleTrackHitsTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_iNCDCHits[iProduct] = int(Variable::trackNCDCHits(selparticles[iProduct]));
    m_iNSVDHits[iProduct] = int(Variable::trackNSVDHits(selparticles[iProduct]));
    m_iNPXDHits[iProduct] = int(Variable::trackNPXDHits(selparticles[iProduct]));
  }
}
