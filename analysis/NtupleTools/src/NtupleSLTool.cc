/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

# include <analysis/NtupleTools/NtupleSLTool.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleSLTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_tree->Branch((strNames[0] + "_lephel").c_str(), &m_helA, (strNames[0] + "_lephel/F").c_str());
  m_tree->Branch((strNames[0] + "_q2").c_str() , &m_q2, (strNames[0] + "_q2/F").c_str());

}

void NtupleSLTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleSLTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 3) {
    B2ERROR("NtupleSLTool::eval - ERROR, you must select exactly 3 particles in the decay 1->had+2+3!\n");
    return;
  }

  //get daughter1
  const Particle*  daughter1 = selparticles[1];

  //get daughter2
  const Particle*  daughter2 = selparticles[2];

  TLorentzVector daughter1Momentum = daughter1->get4Vector();
  TLorentzVector daughter2Momentum = daughter2->get4Vector();

  TLorentzVector motherAMomentum = (daughter1Momentum + daughter2Momentum);
  TVector3       motherABoost    = -(motherAMomentum.BoostVector());

  TLorentzVector daughter1MomentumBoostedA = (daughter1->get4Vector());
  daughter1MomentumBoostedA.Boost(motherABoost);

  m_helA =  cos(daughter1MomentumBoostedA.Angle(motherAMomentum.Vect()));

  m_q2 = motherAMomentum.M2();
}

