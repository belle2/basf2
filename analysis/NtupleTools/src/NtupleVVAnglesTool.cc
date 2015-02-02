/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

# include <analysis/NtupleTools/NtupleVVAnglesTool.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleVVAnglesTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_tree->Branch((strNames[0] + "_hel1").c_str(), &m_helA, (strNames[0] + "_hel1/F").c_str());
  m_tree->Branch((strNames[0] + "_hel2").c_str(), &m_helB, (strNames[0] + "_hel2/F").c_str());
  m_tree->Branch((strNames[0] + "_chiPlanar").c_str(), &m_chi, (strNames[0] + "_chiPlanar/F").c_str());

}

void NtupleVVAnglesTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleVVAnglesTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 5) {
    B2ERROR("NtupleVVAnglesTool::eval - ERROR, you must select exactly 4 particles in the decay 1-> A + B, A->2+3, B->4+5!\n");
    return;
  }

  //get daughter1
  const Particle*  daughter1 = selparticles[1];

  //get daughter2
  const Particle*  daughter2 = selparticles[2];

  //get daughter3
  const Particle*  daughter3 = selparticles[3];

  //get daughter4
  const Particle*  daughter4 = selparticles[4];

  TLorentzVector daughter1Momentum = daughter1->get4Vector();
  TLorentzVector daughter2Momentum = daughter2->get4Vector();
  TLorentzVector daughter3Momentum = daughter3->get4Vector();
  TLorentzVector daughter4Momentum = daughter4->get4Vector();

  TLorentzVector motherAMomentum = (daughter1Momentum + daughter2Momentum);
  TLorentzVector motherBMomentum = (daughter3Momentum + daughter4Momentum);

  TVector3       motherABoost    = -(motherAMomentum.BoostVector());
  TVector3       motherBBoost    = -(motherBMomentum.BoostVector());

  TLorentzVector motherMomentum = (motherAMomentum + motherBMomentum);

  TLorentzVector daughter1MomentumBoostedA = (daughter1->get4Vector());
  daughter1MomentumBoostedA.Boost(motherABoost);
  TLorentzVector daughter3MomentumBoostedB = (daughter3->get4Vector());
  daughter3MomentumBoostedB.Boost(motherBBoost);

  m_helA =  cos(daughter1MomentumBoostedA.Angle(motherAMomentum.Vect()));
  m_helB =  cos(daughter3MomentumBoostedB.Angle(motherBMomentum.Vect()));

  //Boost everything into the Mother frame
  TVector3       motherBoost    = -(motherMomentum.BoostVector());

  TLorentzVector daughter1MomentumBoosted = (daughter1->get4Vector());
  daughter1MomentumBoosted.Boost(motherBoost);
  TLorentzVector daughter2MomentumBoosted = (daughter2->get4Vector());
  daughter2MomentumBoosted.Boost(motherBoost);
  TLorentzVector daughter3MomentumBoosted = (daughter3->get4Vector());
  daughter3MomentumBoosted.Boost(motherBoost);
  TLorentzVector daughter4MomentumBoosted = (daughter4->get4Vector());
  daughter4MomentumBoosted.Boost(motherBoost);
  TLorentzVector motherAMomentumBoosted   = motherAMomentum;
  motherAMomentumBoosted.Boost(motherBoost);
  TLorentzVector motherBMomentumBoosted   = motherBMomentum;
  motherBMomentumBoosted.Boost(motherBoost);

  TVector3 unit_daughter1 = TVector3(daughter1MomentumBoosted.BoostVector()).Unit();
  TVector3 unit_daughter2 = TVector3(daughter2MomentumBoosted.BoostVector()).Unit();
  TVector3 unit_daughter3 = TVector3(daughter3MomentumBoosted.BoostVector()).Unit();
  TVector3 unit_daughter4 = TVector3(daughter4MomentumBoosted.BoostVector()).Unit();

  TVector3  eA = (unit_daughter1.Cross(unit_daughter2)).Unit();
  TVector3  eB = (unit_daughter3.Cross(unit_daughter4)).Unit();
  TVector3  eZ = TVector3(motherBMomentumBoosted.BoostVector()).Unit();

  double cosPhi = (eB.Dot(eA));
  double sinPhi = (eA.Cross(eB)).Dot(eZ);
  double anglesign =  sinPhi > 0.0 ? 1 : -1 ;

  m_chi = acos(cosPhi) * anglesign;



}

