/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald, Phillip Urquijo                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCGenKinematicsTool.h>
#include <framework/datastore/StoreObjPtr.h>
#include <TBranch.h>
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

#include <analysis/VariableManager/Variables.h>

using namespace Belle2;
using namespace std;

int MCGenMax(20);

void NtupleMCGenKinematicsTool::setupTree()
{
  B2INFO("MCGenKinematics option is: " << m_strOption);
  if (m_strOption != "") { MCGenMax = stoi(m_strOption); }

  m_nMCGen = 0;
  m_tree->Branch("nMCGen", &m_nMCGen, "nMCGen/I");

  m_MCGenPDG = new int[MCGenMax];
  TString specifier = "MCGenPDG[";
  specifier += MCGenMax;
  specifier += "]/I";
  m_tree->Branch("MCGenPDG",  &m_MCGenPDG[0], specifier);

  m_MCGenMothIndex = new int[MCGenMax];
  specifier = "MCGenMothIndex[";
  specifier += MCGenMax;
  specifier += "]/I";
  m_tree->Branch("MCGenMothIndex",  &m_MCGenMothIndex[0], specifier);

  m_MCGenP = new float[MCGenMax];
  specifier = "MCGenP[";
  specifier += MCGenMax;
  specifier += "]/F";
  m_tree->Branch("MCGenP", &m_MCGenP[0], specifier);

  m_MCGenTheta = new float[MCGenMax];
  specifier = "MCGenTheta[";
  specifier += MCGenMax;
  specifier += "]/F";
  m_tree->Branch("MCGenTheta", &m_MCGenTheta[0], specifier);

  m_MCGenPhi = new float[MCGenMax];
  specifier = "MCGenPhi[";
  specifier += MCGenMax;
  specifier += "]/F";
  m_tree->Branch("MCGenPhi", &m_MCGenPhi[0], specifier);

}

void NtupleMCGenKinematicsTool::eval(const  Particle*)
{
  StoreArray<MCParticle> mcParticles("MCParticles");

  //..Build relationship between index among stored events and index in MCParticles
  int iGen = 0;
  int storeiGen[200];
  memset(storeiGen, -1, sizeof(storeiGen));
  for (const auto& mcParticle : mcParticles) {
    if (mcParticle.hasStatus(MCParticle::c_PrimaryParticle) && !mcParticle.hasStatus(MCParticle::c_IsVirtual)
        && mcParticle.getArrayIndex() < 200) {
      storeiGen[mcParticle.getArrayIndex()] = iGen;
      iGen++;
    }
  }

  //..Now store the desired quantities
  m_nMCGen = 0;
  for (const auto& mcParticle : mcParticles) {
    iGen = storeiGen[mcParticle.getArrayIndex()];
    int mothIndex = -1;
    MCParticle* mother = mcParticle.getMother();
    if (mother) {
      int mothArrayIndex = mother->getArrayIndex();
      mothIndex = storeiGen[mothArrayIndex];
    }
    if (iGen >= 0 && iGen < MCGenMax) {
      m_MCGenPDG[iGen] = mcParticle.getPDG();
      m_MCGenMothIndex[iGen] = mothIndex;
      TLorentzVector p4 = mcParticle.get4Vector();
      m_MCGenP[iGen] = p4.P();
      m_MCGenTheta[iGen] = p4.Theta();
      m_MCGenPhi[iGen] = p4.Phi();
      m_nMCGen++;
    }
  }
  for (int i = m_nMCGen; i < MCGenMax; i++) {
    m_MCGenPDG[i] = 0;
    m_MCGenMothIndex[i] = 0;
    m_MCGenP[i] = 0.;
    m_MCGenTheta[i] = 0.;
    m_MCGenPhi[i] = 0.;
  }
}
