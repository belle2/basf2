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
#include <TMatrixFSym.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleVertexTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_nDecayProducts = strNames.size();

  m_fDX     = new float[m_nDecayProducts];
  m_fDY     = new float[m_nDecayProducts];
  m_fDZ     = new float[m_nDecayProducts];
  m_fDEX     = new float[m_nDecayProducts];
  m_fDEY     = new float[m_nDecayProducts];
  m_fDEZ     = new float[m_nDecayProducts];
  m_fDRho   = new float[m_nDecayProducts];
  m_fPvalue = new float[m_nDecayProducts];
  m_fProdV     = new float*[m_nDecayProducts];
  m_fProdCov   = new float** [m_nDecayProducts];

  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_X").c_str(), &m_fDX[iProduct], (strNames[iProduct] + "_X/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrX").c_str(), &m_fDEX[iProduct], (strNames[iProduct] + "_ErrX/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Y").c_str(), &m_fDY[iProduct], (strNames[iProduct] + "_Y/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrY").c_str(), &m_fDEY[iProduct], (strNames[iProduct] + "_ErrY/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Z").c_str(), &m_fDZ[iProduct], (strNames[iProduct] + "_Z/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_ErrZ").c_str(), &m_fDEZ[iProduct], (strNames[iProduct] + "_ErrZ/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Rho").c_str(), &m_fDRho[iProduct], (strNames[iProduct] + "_Rho/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_VtxPvalue").c_str(), &m_fPvalue[iProduct], (strNames[iProduct] + "_VtxPvalue/F").c_str());
    m_fProdV[iProduct] = new float[3];
    m_tree->Branch((strNames[iProduct] + "_VtxProd").c_str(), &m_fProdV[iProduct][0], (strNames[iProduct] + "_VtxProd[3]/F").c_str());
    m_fProdCov[iProduct] = new float*[3];
    for (int i = 0; i < 3; i++) m_fProdCov[iProduct][i] = new float[3];
    m_tree->Branch((strNames[iProduct] + "_VtxProdCov").c_str(), &m_fProdCov[iProduct][0][0],
                   (strNames[iProduct] + "_VtxProdCov[3][3]/F").c_str());

  }
}

void NtupleVertexTool::deallocateMemory()
{
  for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
    delete [] m_fProdV[iProduct];
    for (int i = 0; i < 3; i++) {
      delete [] m_fProdCov[iProduct][i];
    }
    delete [] m_fProdCov[iProduct];
  }
  delete [] m_fProdCov;
  delete [] m_fProdV;
  delete m_fPvalue;
  delete m_fDRho;
  delete m_fDEZ;
  delete m_fDEY;
  delete m_fDEX;
  delete m_fDZ;
  delete m_fDY;
  delete m_fDX;
}

void NtupleVertexTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleVertexTool::eval - no Particle found!");
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
    m_fDRho[iProduct]   = TMath::Sqrt(selparticles[iProduct]->getX() * selparticles[iProduct]->getX() + selparticles[iProduct]->getY() *
                                      selparticles[iProduct]->getY());
    m_fPvalue[iProduct] = selparticles[iProduct]->getPValue();

    for (int i = 0; i < 3; i++) {
      m_fProdV[iProduct][i] = -111;
      for (int j = 0; j < 3; j++) {
        m_fProdCov[iProduct][i][j] = -111;
      }
    }

    if (selparticles[iProduct]->hasExtraInfo("prodVertX")) m_fProdV[iProduct][0] = selparticles[iProduct]->getExtraInfo("prodVertX");
    if (selparticles[iProduct]->hasExtraInfo("prodVertY")) m_fProdV[iProduct][1] = selparticles[iProduct]->getExtraInfo("prodVertY");
    if (selparticles[iProduct]->hasExtraInfo("prodVertZ")) m_fProdV[iProduct][2] = selparticles[iProduct]->getExtraInfo("prodVertZ");

    if (selparticles[iProduct]->hasExtraInfo("prodVertSxx")) m_fProdCov[iProduct][0][0] =
        selparticles[iProduct]->getExtraInfo("prodVertSxx");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSxy")) m_fProdCov[iProduct][0][1] =
        selparticles[iProduct]->getExtraInfo("prodVertSxy");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSxz")) m_fProdCov[iProduct][0][2] =
        selparticles[iProduct]->getExtraInfo("prodVertSxz");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSyx")) m_fProdCov[iProduct][1][0] =
        selparticles[iProduct]->getExtraInfo("prodVertSyx");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSyy")) m_fProdCov[iProduct][1][1] =
        selparticles[iProduct]->getExtraInfo("prodVertSyy");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSyz")) m_fProdCov[iProduct][1][2] =
        selparticles[iProduct]->getExtraInfo("prodVertSyz");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSzx")) m_fProdCov[iProduct][2][0] =
        selparticles[iProduct]->getExtraInfo("prodVertSzx");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSzy")) m_fProdCov[iProduct][2][1] =
        selparticles[iProduct]->getExtraInfo("prodVertSzy");
    if (selparticles[iProduct]->hasExtraInfo("prodVertSzz")) m_fProdCov[iProduct][2][2] =
        selparticles[iProduct]->getExtraInfo("prodVertSzz");

  }
}




