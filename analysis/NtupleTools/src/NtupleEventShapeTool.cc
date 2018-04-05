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
#include <analysis/dataobjects/EventShape.h>

#include <TBranch.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleEventShapeTool::setupTree()
{
  m_tree->Branch("thrustValue", &m_fThrustValue, "thrustValue/F");
  m_tree->Branch("thrustVector", &m_fThrustVector[0], "thrustVector[3]/F");
  m_tree->Branch("missingMomentum", &m_fMissingMomentum[0], "missingMomentum[3]/F");
  m_tree->Branch("missingMomentumCMS", &m_fMissingMomentumCMS[0], "missingMomentumCMS[3]/F");
  m_tree->Branch("missingEnergy", &m_fMissingEnergy, "missingEnergy/F");
  m_tree->Branch("missingMass2", &m_fMissingMass2, "missingMass2/F");
  m_tree->Branch("visibleEnergy", &m_fVisibleEnergy, "visibleEnergy/F");

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

  StoreObjPtr<EventShape> evtShape;
  if (evtShape) {
    m_fThrustValue = evtShape->getThrust();
    TVector3 thr = evtShape->getThrustAxis();
    TVector3 missing = evtShape->getMissingMomentum();
    TVector3 missingCMS = evtShape->getMissingMomentumCMS();
    m_fMissingEnergy = evtShape->getMissingEnergyCMS();
    m_fMissingMass2 = evtShape->getMissingMass2();
    m_fVisibleEnergy = evtShape->getVisibleEnergy();
    for (int i = 0; i < 3; i++) {
      m_fThrustVector[i] = thr(i);
      m_fMissingMomentum[i] = missing(i);
      m_fMissingMomentumCMS[i] = missingCMS(i);
    }
  } else {
    B2WARNING("Variables not found, did you forget to run EventShapeModule?");
    m_fThrustValue = std::numeric_limits<float>::quiet_NaN();
    for (int i = 0; i < 3; i++) {
      m_fThrustVector[i] = std::numeric_limits<float>::quiet_NaN();
      m_fMissingMomentum[i] = std::numeric_limits<float>::quiet_NaN();
      m_fMissingMomentumCMS[i] = std::numeric_limits<float>::quiet_NaN();
      m_fMissingEnergy = std::numeric_limits<float>::quiet_NaN();
      m_fMissingMass2 = std::numeric_limits<float>::quiet_NaN();
      m_fVisibleEnergy = std::numeric_limits<float>::quiet_NaN();
    }
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  int nDecayProducts = selparticles.size();
  for (int iProduct = 0; iProduct < nDecayProducts; iProduct++) {
    m_fCosToThrust[iProduct] = Variable::cosToThrustOfEvent(selparticles[iProduct]);
  }
}
