/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleEventShapeTool.h>
#include <TBranch.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/EventVariables.h>

using namespace Belle2;
using namespace std;

void NtupleEventShapeTool::setupTree()
{
  m_tree->Branch("ThrustValue", &m_fThrustValue, "ThrustValue/F");
  m_tree->Branch("ThrustVector", &m_fThrustVector[0], "ThrustVector[3]/F");

  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  B2INFO("nDecayProducts = " << nDecayProducts);

  m_fCosToThrust = new float[nDecayProducts];

  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_tree->Branch((strNames[iProduct] + "_CosToThrust").c_str(),  &m_fCosToThrust[iProduct],
                   (strNames[iProduct] + "_CosToThrust/F").c_str());
  }
}

void NtupleEventShapeTool::deallocateMemory()
{
  m_fThrustValue = 0;
  for (int i = 0; i < 3; i++) {m_fThrustVector[i] = -100;}
  delete [] m_fThrustValue;
  delete [] m_fCosToThrust;
}

void NtupleEventShapeTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleEventShapeTool::eval - no Particle found!");
    return;
  }

  m_fThrustValue = EventVariable::thrustOfEvent;
//  for (int i = 0; i < 3; i++) {m_fThrustVector[i] = EventVariable:: };

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fCosToThrust[iProduct] = Variable::cosToEvtThrust(selparticles[iProduct]);
  }
}
