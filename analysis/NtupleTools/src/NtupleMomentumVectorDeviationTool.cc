/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Anze Zupanc                                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMomentumVectorDeviationTool.h>
#include <cmath>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleMomentumVectorDeviationTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  m_tree->Branch((strNames[1] + "_cosMomVert").c_str(), &m_cosAngle, (strNames[1] + "_cosMomVert/F").c_str());
}

void NtupleMomentumVectorDeviationTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleMomentumVectorDeviationTool::eval - ERROR, no Particle found!\n");
    return;
  }


  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 2) {
    printf("NtupleMomentumVectorDeviationTool::eval - ERROR, you must to select exactly 2 particles in the decay (mother and daughter)!\n");
    return;
  }

  //get the MOTHER
  const Particle*  mother = selparticles[0];

  //get the DAUGHTER
  const Particle*  daughter = selparticles[1];

  m_cosAngle = std::cos((daughter->getVertex() - mother->getVertex()).Angle(daughter->getMomentum()));
}

