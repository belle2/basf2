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

#include <analysis/NtupleTools/NtupleEventKinematicsTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/dataobjects/EventKinematics.h>

#include <TBranch.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleEventKinematicsTool::setupTree()
{
  m_tree->Branch("missingMomentum", &m_fMissingMomentum[0], "missingMomentum[3]/F");
  m_tree->Branch("missingMomentumCMS", &m_fMissingMomentumCMS[0], "missingMomentumCMS[3]/F");
  m_tree->Branch("missingEnergyCMS", &m_fMissingEnergyCMS, "missingEnergyCMS/F");
  m_tree->Branch("missingMass2", &m_fMissingMass2, "missingMass2/F");
  m_tree->Branch("visibleEnergyCMS", &m_fVisibleEnergyCMS, "visibleEnergyCMS/F");
  m_tree->Branch("TotalPhotonsEnergy", &m_fTotalPhotonsEnergy, "TotalPhotonsEnergy/F");

  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  int nDecayProducts = strNames.size();
  B2INFO("nDecayProducts = " << nDecayProducts);
}

void NtupleEventKinematicsTool::deallocateMemory()
{
}

void NtupleEventKinematicsTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleEventKinematicsTool::eval - no Particle found!");
    return;
  }

  StoreObjPtr<EventKinematics> evtKinematics;
  if (evtKinematics) {
    TVector3 missing = evtKinematics->getMissingMomentum();
    TVector3 missingCMS = evtKinematics->getMissingMomentumCMS();
    for (int i = 0; i < 3; i++) {
      m_fMissingMomentum[i] = missing(i);
      m_fMissingMomentumCMS[i] = missingCMS(i);
    }
    m_fMissingEnergyCMS = evtKinematics->getMissingEnergyCMS();
    m_fMissingMass2 = evtKinematics->getMissingMass2();
    m_fVisibleEnergyCMS = evtKinematics->getVisibleEnergyCMS();
    m_fTotalPhotonsEnergy = evtKinematics->getTotalPhotonsEnergy();
  } else {
    B2WARNING("Variables not found, did you forget to run EventKinematicsModule?");
    for (int i = 0; i < 3; i++) {
      m_fMissingMomentum[i] = std::numeric_limits<float>::quiet_NaN();
      m_fMissingMomentumCMS[i] = std::numeric_limits<float>::quiet_NaN();
    }
    m_fMissingEnergyCMS = std::numeric_limits<float>::quiet_NaN();
    m_fMissingMass2 = std::numeric_limits<float>::quiet_NaN();
    m_fVisibleEnergyCMS = std::numeric_limits<float>::quiet_NaN();
    m_fTotalPhotonsEnergy = std::numeric_limits<float>::quiet_NaN();
  }
}
