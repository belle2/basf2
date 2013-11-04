/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleVertexTool.h>
#include <analysis/utility/PSelectorFunctions.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>
#include <analysis/utility/physics.h>

void NtupleVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  int nDecayProducts = strNames.size();

  m_fDX     = new float[nDecayProducts];
  m_fDY     = new float[nDecayProducts];
  m_fDZ     = new float[nDecayProducts];
  m_fDRho   = new float[nDecayProducts];
  m_fPvalue = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_X").c_str(), &m_fDX[iProduct], (strNames[iProduct] + "_X/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Y").c_str(), &m_fDY[iProduct], (strNames[iProduct] + "_Y/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Z").c_str(), &m_fDZ[iProduct], (strNames[iProduct] + "_Z/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Rho").c_str(), &m_fDRho[iProduct], (strNames[iProduct] + "_Rho/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Pval").c_str(), &m_fPvalue[iProduct], (strNames[iProduct] + "_Pval/F").c_str());
  }
}

void NtupleVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleVertexTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fDX[iProduct]     = analysis::particleDX(selparticles[iProduct]);
    m_fDY[iProduct]     = analysis::particleDY(selparticles[iProduct]);
    m_fDZ[iProduct]     = analysis::particleDZ(selparticles[iProduct]);
    m_fDRho[iProduct]   = analysis::particleDRho(selparticles[iProduct]);
    m_fPvalue[iProduct] = analysis::particlePvalue(selparticles[iProduct]);
  }
}




