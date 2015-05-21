/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Matic Lubej                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleErrMatrixTool.h>
#include <TBranch.h>
#include <TMatrixFSym.h>


using namespace Belle2;
using namespace std;

void NtupleErrMatrixTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_err00 = new float[nDecayProducts];
  m_err10 = new float[nDecayProducts];
  m_err11 = new float[nDecayProducts];
  m_err20 = new float[nDecayProducts];
  m_err21 = new float[nDecayProducts];
  m_err22 = new float[nDecayProducts];
  m_err30 = new float[nDecayProducts];
  m_err31 = new float[nDecayProducts];
  m_err32 = new float[nDecayProducts];
  m_err33 = new float[nDecayProducts];
  m_err40 = new float[nDecayProducts];
  m_err41 = new float[nDecayProducts];
  m_err42 = new float[nDecayProducts];
  m_err43 = new float[nDecayProducts];
  m_err44 = new float[nDecayProducts];
  m_err50 = new float[nDecayProducts];
  m_err51 = new float[nDecayProducts];
  m_err52 = new float[nDecayProducts];
  m_err53 = new float[nDecayProducts];
  m_err54 = new float[nDecayProducts];
  m_err55 = new float[nDecayProducts];
  m_err60 = new float[nDecayProducts];
  m_err61 = new float[nDecayProducts];
  m_err62 = new float[nDecayProducts];
  m_err63 = new float[nDecayProducts];
  m_err64 = new float[nDecayProducts];
  m_err65 = new float[nDecayProducts];
  m_err66 = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_Err00").c_str(), &m_err00[iProduct], (strNames[iProduct] + "_Err00/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err10").c_str(), &m_err10[iProduct], (strNames[iProduct] + "_Err10/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err11").c_str(), &m_err11[iProduct], (strNames[iProduct] + "_Err11/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err20").c_str(), &m_err20[iProduct], (strNames[iProduct] + "_Err20/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err21").c_str(), &m_err21[iProduct], (strNames[iProduct] + "_Err21/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err22").c_str(), &m_err22[iProduct], (strNames[iProduct] + "_Err22/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err30").c_str(), &m_err30[iProduct], (strNames[iProduct] + "_Err30/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err31").c_str(), &m_err31[iProduct], (strNames[iProduct] + "_Err31/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err32").c_str(), &m_err32[iProduct], (strNames[iProduct] + "_Err32/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err33").c_str(), &m_err33[iProduct], (strNames[iProduct] + "_Err33/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err40").c_str(), &m_err40[iProduct], (strNames[iProduct] + "_Err40/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err41").c_str(), &m_err41[iProduct], (strNames[iProduct] + "_Err41/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err42").c_str(), &m_err42[iProduct], (strNames[iProduct] + "_Err42/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err43").c_str(), &m_err43[iProduct], (strNames[iProduct] + "_Err43/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err44").c_str(), &m_err44[iProduct], (strNames[iProduct] + "_Err44/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err50").c_str(), &m_err50[iProduct], (strNames[iProduct] + "_Err50/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err51").c_str(), &m_err51[iProduct], (strNames[iProduct] + "_Err51/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err52").c_str(), &m_err52[iProduct], (strNames[iProduct] + "_Err52/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err53").c_str(), &m_err53[iProduct], (strNames[iProduct] + "_Err53/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err54").c_str(), &m_err54[iProduct], (strNames[iProduct] + "_Err54/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err55").c_str(), &m_err55[iProduct], (strNames[iProduct] + "_Err55/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err60").c_str(), &m_err60[iProduct], (strNames[iProduct] + "_Err60/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err61").c_str(), &m_err61[iProduct], (strNames[iProduct] + "_Err61/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err62").c_str(), &m_err62[iProduct], (strNames[iProduct] + "_Err62/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err63").c_str(), &m_err63[iProduct], (strNames[iProduct] + "_Err63/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err64").c_str(), &m_err64[iProduct], (strNames[iProduct] + "_Err64/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err65").c_str(), &m_err65[iProduct], (strNames[iProduct] + "_Err65/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Err66").c_str(), &m_err66[iProduct], (strNames[iProduct] + "_Err66/F").c_str());
  }
}

void NtupleErrMatrixTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleErrMatrixTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    TMatrixFSym ErrM = selparticles[iProduct]->getMomentumVertexErrorMatrix();

    m_err00[iProduct] = ErrM[0][0];
    m_err10[iProduct] = ErrM[1][0];
    m_err11[iProduct] = ErrM[1][1];
    m_err20[iProduct] = ErrM[2][0];
    m_err21[iProduct] = ErrM[2][1];
    m_err22[iProduct] = ErrM[2][2];
    m_err30[iProduct] = ErrM[3][0];
    m_err31[iProduct] = ErrM[3][1];
    m_err32[iProduct] = ErrM[3][2];
    m_err33[iProduct] = ErrM[3][3];
    m_err40[iProduct] = ErrM[4][0];
    m_err41[iProduct] = ErrM[4][1];
    m_err42[iProduct] = ErrM[4][2];
    m_err43[iProduct] = ErrM[4][3];
    m_err44[iProduct] = ErrM[4][4];
    m_err50[iProduct] = ErrM[5][0];
    m_err51[iProduct] = ErrM[5][1];
    m_err52[iProduct] = ErrM[5][2];
    m_err53[iProduct] = ErrM[5][3];
    m_err54[iProduct] = ErrM[5][4];
    m_err55[iProduct] = ErrM[5][5];
    m_err60[iProduct] = ErrM[6][0];
    m_err61[iProduct] = ErrM[6][1];
    m_err62[iProduct] = ErrM[6][2];
    m_err63[iProduct] = ErrM[6][3];
    m_err64[iProduct] = ErrM[6][4];
    m_err65[iProduct] = ErrM[6][5];
    m_err66[iProduct] = ErrM[6][6];

  }
}
