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
  m_fPIDd = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_kaonID").c_str(),  &m_fPIDk[iProduct], (strNames[iProduct] + "_kaonID/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_pionID").c_str(), &m_fPIDpi[iProduct], (strNames[iProduct] + "_pionID/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_electronID").c_str(),  &m_fPIDe[iProduct], (strNames[iProduct] + "_electronID/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_muonID").c_str(), &m_fPIDmu[iProduct], (strNames[iProduct] + "_muonID/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_protonID").c_str(),  &m_fPIDp[iProduct], (strNames[iProduct] + "_protonID/F").c_str());
    m_tree->Branch((strNames[iProduct] + "_deuteronID").c_str(),  &m_fPIDd[iProduct], (strNames[iProduct] + "_deuteronID/F").c_str());
  }
}

void NtuplePIDTool::deallocateMemory()
{
  delete [] m_fPIDk;
  delete [] m_fPIDpi;
  delete [] m_fPIDe;
  delete [] m_fPIDmu;
  delete [] m_fPIDp;
  delete [] m_fPIDd;
}

void NtuplePIDTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtuplePIDTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {

    m_fPIDk[iProduct] = Variable::kaonID(selparticles[iProduct]);
    m_fPIDpi[iProduct] = Variable::pionID(selparticles[iProduct]);
    m_fPIDe[iProduct] = Variable::electronID(selparticles[iProduct]);
    m_fPIDmu[iProduct] = Variable::muonID(selparticles[iProduct]);
    m_fPIDp[iProduct] = Variable::protonID(selparticles[iProduct]);
    m_fPIDd[iProduct] = Variable::deuteronID(selparticles[iProduct]);

  }
}
