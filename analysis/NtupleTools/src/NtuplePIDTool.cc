/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtuplePIDTool.h>
#include <analysis/VariableManager/PIDVariables.h>
#include <framework/gearbox/Const.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtuplePIDTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  m_fPIDk = new float[nDecayProducts];
  m_fPIDpi = new float[nDecayProducts];
  m_fPIDe = new float[nDecayProducts];
  m_fPIDmu = new float[nDecayProducts];
  m_fPIDp = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_PIDk").c_str(),  &m_fPIDk[iProduct], (strNames[iProduct] + "_PIDk/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_PIDpi").c_str(), &m_fPIDpi[iProduct], (strNames[iProduct] + "_PIDpi/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_PIDe").c_str(),  &m_fPIDe[iProduct], (strNames[iProduct] + "_PIDe/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_PIDmu").c_str(), &m_fPIDmu[iProduct], (strNames[iProduct] + "_PIDmu/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_PIDp").c_str(),  &m_fPIDp[iProduct], (strNames[iProduct] + "_PIDp/F").c_str());
  }
}

void NtuplePIDTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtuplePIDTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    m_fPIDk[iProduct] = Variable::particleKaonId(selparticles[iProduct]);
    m_fPIDpi[iProduct] = Variable::particlePionId(selparticles[iProduct]);
    m_fPIDe[iProduct] = Variable::particleElectronId(selparticles[iProduct]);
    m_fPIDmu[iProduct] = Variable::particleMuonId(selparticles[iProduct]);
    m_fPIDp[iProduct] = Variable::particleProtonId(selparticles[iProduct]);



  }
}
