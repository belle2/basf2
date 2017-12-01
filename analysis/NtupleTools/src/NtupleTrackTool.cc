/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo, Anze Zupanc           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleTrackTool.h>
#include <analysis/VariableManager/TrackVariables.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleTrackTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_fD0 = new float[nDecayProducts];
  m_ferrD0 = new float[nDecayProducts];
  m_fZ0 = new float[nDecayProducts];
  m_ferrZ0 = new float[nDecayProducts];
  m_fTrPval = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_d0").c_str(), &m_fD0[iProduct], (strNames[iProduct] + "_d0/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_d0err").c_str(), &m_ferrD0[iProduct], (strNames[iProduct] + "_d0err/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_z0").c_str(), &m_fZ0[iProduct], (strNames[iProduct] + "_z0/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_z0err").c_str(), &m_ferrZ0[iProduct], (strNames[iProduct] + "_z0err/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_TrPval").c_str(), &m_fTrPval[iProduct], (strNames[iProduct] + "_TrPval/F").c_str());
  }
}

void NtupleTrackTool::deallocateMemory()
{
  delete [] m_fD0;
  delete [] m_ferrD0;
  delete [] m_fZ0;
  delete [] m_ferrZ0;
  delete [] m_fTrPval;
}

void NtupleTrackTool::eval(const Particle* particle)
{

  if (!particle) {
    B2ERROR("NtupleTrackTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fD0[iProduct]     = Variable::trackD0(selparticles[iProduct]);
    m_ferrD0[iProduct]     = Variable::trackD0Error(selparticles[iProduct]);
    m_fZ0[iProduct]     = Variable::trackZ0(selparticles[iProduct]);
    m_ferrZ0[iProduct]     = Variable::trackZ0Error(selparticles[iProduct]);
    m_fTrPval[iProduct] = Variable::trackPValue(selparticles[iProduct]);
  }
}
