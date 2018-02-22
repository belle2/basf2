/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hikari Hirata                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/NtupleTools/NtupleAngularTool.h>
#include <analysis/VariableManager/PIDVariables.h>
#include <analysis/VariableManager/AngularVariablesModule.h>
#include <framework/gearbox/Const.h>
#include <TBranch.h>

using namespace Belle2;
using namespace std;

void NtupleAngularTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  // m_nDecayProducts = strNames.size();
  //  m_fAngular = new float[m_nDecayProducts];

  //for (int iProduct = 0; iProduct < m_nDecayProducts; iProduct++) {
  m_tree->Branch((strNames[0] + "_Angular").c_str(),  &m_fAngular, (strNames[0] + "_Angular/F").c_str());
  //    m_tree->Branch((strNames[iProduct] + "_Angular").c_str(),  &m_fAngular[iProduct], (strNames[iProduct] + "_Angular/F").c_str());
  //  }
}
/*
void NtupleAngularTool::deallocateMemory()
{
  delete [] m_fAngular;
}
*/
void NtupleAngularTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleAngularTool::eval - no Particle found!");
    return;
  }


  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  vector<double> index = {0, 1};
  //  m_fAngular = Variable::helicityAngle(particle, index);
  m_fAngular = Variable::helicityAngle(particle, index);
  /*
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fAngular[iProduct] = Variable::kaonID(selparticles[iProduct]);
  }
  */
}
