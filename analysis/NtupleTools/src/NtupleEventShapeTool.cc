/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Ami Rostomyan, Michel H. Villanueva                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <analysis/NtupleTools/NtupleEventShapeTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/ThrustOfEvent.h>

#include <TBranch.h>
#include <TVector3.h>

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
  delete [] m_fCosToThrust;
}

void NtupleEventShapeTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleEventShapeTool::eval - no Particle found!");
    return;
  }

  StoreObjPtr<ThrustOfEvent> thrust;
  if (thrust) {
    m_fThrustValue = thrust->getThrust();
    TVector3 thr = thrust->getThrustAxis();
    for (int i = 0; i < 3; i++) {
      m_fThrustVector[i] = thr(i);
    }
  } else {
    B2WARNING("Thrust of event not found, did you forget to run ThrustOfEventModule?");
    m_fThrustValue = std::numeric_limits<float>::quiet_NaN();
    for (int i = 0; i < 3; i++) {
      m_fThrustVector[i] = std::numeric_limits<float>::quiet_NaN();
    }
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fCosToThrust[iProduct] = Variable::cosToThrustOfEvent(selparticles[iProduct]);
  }
}
