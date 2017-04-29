/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCGenCMSKinematicsTool.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <mdst/dataobjects/MCParticle.h>

#include <analysis/VariableManager/Variables.h>

using namespace Belle2;
using namespace std;

int MCGenCMSMax(20);

void NtupleMCGenCMSKinematicsTool::setupTree()
{
  B2INFO("MCGenCMSKinematics option is: " << m_strOption);
  if (m_strOption != "") { MCGenCMSMax = stoi(m_strOption); }

  m_nMCGenCMS = 0;
  m_tree->Branch("nMCGenCMS", &m_nMCGenCMS, "nMCGenCMS/I");

  m_MCGenCMSPDG = new int[MCGenCMSMax];
  TString specifier = "MCGenCMSPDG[";
  specifier += MCGenCMSMax;
  specifier += "]/I";
  m_tree->Branch("MCGenCMSPDG",  &m_MCGenCMSPDG[0], specifier);

  m_MCGenCMSMothIndex = new int[MCGenCMSMax];
  specifier = "MCGenCMSMothIndex[";
  specifier += MCGenCMSMax;
  specifier += "]/I";
  m_tree->Branch("MCGenCMSMothIndex",  &m_MCGenCMSMothIndex[0], specifier);

  m_MCGenCMSP = new float[MCGenCMSMax];
  specifier = "MCGenCMSP[";
  specifier += MCGenCMSMax;
  specifier += "]/F";
  m_tree->Branch("MCGenCMSP", &m_MCGenCMSP[0], specifier);

  m_MCGenCMSTheta = new float[MCGenCMSMax];
  specifier = "MCGenCMSTheta[";
  specifier += MCGenCMSMax;
  specifier += "]/F";
  m_tree->Branch("MCGenCMSTheta", &m_MCGenCMSTheta[0], specifier);

  m_MCGenCMSPhi = new float[MCGenCMSMax];
  specifier = "MCGenCMSPhi[";
  specifier += MCGenCMSMax;
  specifier += "]/F";
  m_tree->Branch("MCGenCMSPhi", &m_MCGenCMSPhi[0], specifier);

}

void NtupleMCGenCMSKinematicsTool::eval(const  Particle*)
{
  PCmsLabTransform boostrotate;
  StoreArray<MCParticle> mcParticles("MCParticles");

  //..Build relationship between index among stored events and index in MCParticles
  int iGenCMS = 0;
  int storeiGenCMS[200];
  memset(storeiGenCMS, -1, sizeof(storeiGenCMS));
  for (const auto& mcParticle : mcParticles) {
    if (mcParticle.hasStatus(MCParticle::c_PrimaryParticle) && !mcParticle.hasStatus(MCParticle::c_IsVirtual)
        && mcParticle.getArrayIndex() < 200) {
      storeiGenCMS[mcParticle.getArrayIndex()] = iGenCMS;
      iGenCMS++;
    }
  }

  //..Now store the desired quantities
  m_nMCGenCMS = 0;
  for (const auto& mcParticle : mcParticles) {
    iGenCMS = storeiGenCMS[mcParticle.getArrayIndex()];
    int mothIndex = -1;
    MCParticle* mother = mcParticle.getMother();
    if (mother) {
      int mothArrayIndex = mother->getArrayIndex();
      mothIndex = storeiGenCMS[mothArrayIndex];
    }
    if (iGenCMS >= 0 && iGenCMS < MCGenCMSMax) {
      m_MCGenCMSPDG[iGenCMS] = mcParticle.getPDG();
      m_MCGenCMSMothIndex[iGenCMS] = mothIndex;
      TLorentzVector p4 = mcParticle.get4Vector();
      TLorentzVector p4CMS = boostrotate.rotateLabToCms() * p4;
      m_MCGenCMSP[iGenCMS] = p4CMS.P();
      m_MCGenCMSTheta[iGenCMS] = p4CMS.Theta();
      m_MCGenCMSPhi[iGenCMS] = p4CMS.Phi();
      m_nMCGenCMS++;
    }
  }
  for (int i = m_nMCGenCMS; i < MCGenCMSMax; i++) {
    m_MCGenCMSPDG[i] = 0;
    m_MCGenCMSMothIndex[i] = 0;
    m_MCGenCMSP[i] = 0.;
    m_MCGenCMSTheta[i] = 0.;
    m_MCGenCMSPhi[i] = 0.;
  }
}
