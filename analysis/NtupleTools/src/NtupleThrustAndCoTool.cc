/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleThrustAndCoTool.h>
#include <TBranch.h>
#include <analysis/VariableManager/Variables.h>

using namespace Belle2;
using namespace std;

void NtupleThrustAndCoTool::setupTree()
{
  m_tree->Branch("ThrustValue", &m_fThrustValue, "ThrustValue/F");
  m_tree->Branch("ThrustVector", &m_fThrustVector[0], "ThrustVector[3]/F");

  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  B2INFO("nDecayProducts = " << nDecayProducts);

  m_fCosThrust = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_CosThrust").c_str(),  &m_fCosThrust[iProduct],
                   (strNames[iProduct] + "_CosThrust/F").c_str());
  }
}

void NtupleThrustAndCoTool::deallocateMemory()
{
  m_fThrustValue = 0;
  for (int i = 0; i < 3; i++) {m_fThrustVector[i] = -100;}
  delete [] m_fCosThrust;
}

void NtupleThrustAndCoTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleThrustAndCoTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    B2INFO("Here we will give values to the variables");
//     m_fPIDk[iProduct] = Variable::kaonID(selparticles[iProduct]);
  }
}
