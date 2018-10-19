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

  m_tree->Branch((strNames[1] + strNames[2] + "_hel").c_str(), &m_helA, (strNames[1] + strNames[2] + "_hel/F").c_str());

}

void NtupleHelicityTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleHelicityTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 3) {
    B2ERROR("NtupleHelicityTool::eval - ERROR, you must select exactly 3 particles: mother and two daughters of a given particle.\n");
    return;
  }

  const Particle* mother = selparticles[0];
  const Particle* daughter1 = selparticles[1];
  const Particle* daughter2 = selparticles[2];
  TLorentzVector daughterP4 = daughter1->get4Vector();
  TLorentzVector thisP4 = daughterP4 + daughter2->get4Vector();
  TLorentzVector motherP4 = mother->get4Vector();
  TVector3 boost = -(thisP4.BoostVector());
  daughterP4.Boost(boost);
  motherP4.Boost(boost);
  TVector3 daughterP3 = daughterP4.Vect();
  TVector3 motherP3 = motherP4.Vect();
  m_helA = - daughterP3.Dot(motherP3) / daughterP3.Mag() / motherP3.Mag();
}

