/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleDalitzTool.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleDalitzTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_tree->Branch((strNames[0] + strNames[1] + "_m12").c_str(), &m_m12, (strNames[0] + strNames[1] + "_m12/F").c_str());
  m_tree->Branch((strNames[0] + strNames[2] + "_m13").c_str(), &m_m13, (strNames[0] + strNames[2] + "_m13/F").c_str());
  m_tree->Branch((strNames[1] + strNames[2] + "_m23").c_str(), &m_m23, (strNames[1] + strNames[2] + "_m23/F").c_str());

}

void NtupleDalitzTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleDalitzTool::eval - ERROR, no Particle found!\n");
    return;
  }


  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 3) {
    printf("NtupleDalitzTool::eval - ERROR, you must select exactly 3 particles in the decay (daughter1, daughter2, daughter3)!\n");
    return;
  }

  //get daughter1
  const Particle*  daughter1 = selparticles[0];

  //get daughter2
  const Particle*  daughter2 = selparticles[1];

  //get daughter3
  const Particle*  daughter3 = selparticles[2];

  m_m12 = (daughter1->get4Vector() + daughter2->get4Vector()).M();
  m_m13 = (daughter1->get4Vector() + daughter3->get4Vector()).M();
  m_m23 = (daughter2->get4Vector() + daughter3->get4Vector()).M();

}

