/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleRecoilKinematicsTool.h>

#include <analysis/VariableManager/Variables.h>

#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleRecoilKinematicsTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();

  m_recoilP3  = new float*[nDecayProducts];
  m_recoilP   = new float[nDecayProducts];
  m_recoilE   = new float[nDecayProducts];
  m_recoilM   = new float[nDecayProducts];
  m_recoilMM2 = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_recoilP3[iProduct] = new float[3];
    m_tree->Branch((strNames[iProduct] + "_P3recoil").c_str(),  &m_recoilP3[iProduct][0],
                   (strNames[iProduct] + "_P3recoil[3]/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Precoil").c_str(),  &m_recoilP[iProduct], (strNames[iProduct] + "_Precoil/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Erecoil").c_str(),  &m_recoilE[iProduct], (strNames[iProduct] + "_Erecoil/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_Mrecoil").c_str(),  &m_recoilM[iProduct], (strNames[iProduct] + "_Mrecoil/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_M2recoil").c_str(), &m_recoilMM2[iProduct], (strNames[iProduct] + "_M2recoil/F").c_str());
  }
}

void NtupleRecoilKinematicsTool::eval(const Particle* particle)
{
  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);

  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_recoilP3[iProduct][0]  = Variable::recoilPx(selparticles[iProduct]);
    m_recoilP3[iProduct][1]  = Variable::recoilPy(selparticles[iProduct]);
    m_recoilP3[iProduct][2]  = Variable::recoilPz(selparticles[iProduct]);
    m_recoilP[iProduct]   = Variable::recoilMomentum(selparticles[iProduct]);
    m_recoilE[iProduct]   = Variable::recoilEnergy(selparticles[iProduct]);
    m_recoilM[iProduct]   = Variable::recoilMass(selparticles[iProduct]);
    m_recoilMM2[iProduct] = Variable::recoilMassSquared(selparticles[iProduct]);
  }
}
