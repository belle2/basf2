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
#include <analysis/VariableManager/Variables.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  int nDecayProducts = strNames.size();

  m_fDX     = new float[nDecayProducts];
  m_fDY     = new float[nDecayProducts];
  m_fDZ     = new float[nDecayProducts];
  m_fDEX     = new float[nDecayProducts];
  m_fDEY     = new float[nDecayProducts];
  m_fDEZ     = new float[nDecayProducts];
  m_fDRho   = new float[nDecayProducts];
  m_fPvalue = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_X").c_str(), &m_fDX[iProduct], (strNames[iProduct] + "_X/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrX").c_str(), &m_fDEX[iProduct], (strNames[iProduct] + "_ErrX/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Y").c_str(), &m_fDY[iProduct], (strNames[iProduct] + "_Y/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrY").c_str(), &m_fDEY[iProduct], (strNames[iProduct] + "_ErrY/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Z").c_str(), &m_fDZ[iProduct], (strNames[iProduct] + "_Z/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrZ").c_str(), &m_fDEZ[iProduct], (strNames[iProduct] + "_ErrZ/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Rho").c_str(), &m_fDRho[iProduct], (strNames[iProduct] + "_Rho/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_VtxPvalue").c_str(), &m_fPvalue[iProduct], (strNames[iProduct] + "_VtxPvalue/F").c_str());
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
    m_fDX[iProduct]     = selparticles[iProduct]->getX();
    m_fDY[iProduct]     = selparticles[iProduct]->getY();
    m_fDZ[iProduct]     = selparticles[iProduct]->getZ();
    m_fDEX[iProduct]     = TMath::Sqrt(selparticles[iProduct]->getVertexErrorMatrix()[0][0]);
    m_fDEY[iProduct]     = TMath::Sqrt(selparticles[iProduct]->getVertexErrorMatrix()[1][1]);
    m_fDEZ[iProduct]     = TMath::Sqrt(selparticles[iProduct]->getVertexErrorMatrix()[2][2]);
    m_fDRho[iProduct]   = TMath::Sqrt(selparticles[iProduct]->getX() * selparticles[iProduct]->getX() + selparticles[iProduct]->getY() * selparticles[iProduct]->getY());
    m_fPvalue[iProduct] = selparticles[iProduct]->getPValue();
  }
}




