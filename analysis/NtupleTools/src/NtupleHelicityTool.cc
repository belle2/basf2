/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

# include <analysis/NtupleTools/NtupleHelicityTool.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleHelicityTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_tree->Branch((strNames[0] + strNames[1] + "_hel").c_str(), &m_helA, (strNames[0] + strNames[1] + "_hel/F").c_str());

}

void NtupleHelicityTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleHelicityTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 2) {
    B2ERROR("NtupleHelicityTool::eval - you must select exactly 2 particles in the decay A->1+2!");
    return;
  }

  //get daughter1
  const Particle*  daughter1 = selparticles[0];

  //get daughter2
  const Particle*  daughter2 = selparticles[1];

  TLorentzVector daughter1Momentum = daughter1->get4Vector();
  TLorentzVector daughter2Momentum = daughter2->get4Vector();

  TLorentzVector motherAMomentum = (daughter1Momentum + daughter2Momentum);
  TVector3       motherABoost    = -(motherAMomentum.BoostVector());

  TLorentzVector daughter1MomentumBoostedA = (daughter1->get4Vector());
  daughter1MomentumBoostedA.Boost(motherABoost);

  m_helA =  cos(daughter1MomentumBoostedA.Angle(motherAMomentum.Vect()));


}

