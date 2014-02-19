/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleECLPi0Tool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <TBranch.h>

void NtupleECLPi0Tool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_invM     = new float[nDecayProducts];
  m_pValue   = new float[nDecayProducts];
  m_cosTheta = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_invM").c_str(),   &m_invM[iProduct], (strNames[iProduct] + "_invM/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_pValue").c_str(), &m_pValue[iProduct], (strNames[iProduct] + "_pValue/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_cTheta").c_str(), &m_cosTheta[iProduct], (strNames[iProduct] + "_cTheta/F").c_str());
  }
}

void NtupleECLPi0Tool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleECLPi0Tool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_invM[iProduct]     = analysis::particleInvariantMass(selparticles[iProduct]);
    m_pValue[iProduct]   = selparticles[iProduct]->getPValue();
    m_cosTheta[iProduct] = analysis::particleDecayAngle(selparticles[iProduct]);
  }
}
