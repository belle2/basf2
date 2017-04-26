/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Giulia Casarosa                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleMCFlightInfoTool.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

using namespace Belle2;
using namespace std;

void NtupleMCFlightInfoTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  //flight distance
  m_tree->Branch((strNames[1] + "_MCFD").c_str(), &m_fD, (strNames[1] + "_MCFD/F").c_str());

  //flight time
  m_tree->Branch((strNames[1] + "_MCFT").c_str(), &m_fT, (strNames[1] + "_MCFT/F").c_str());

}

void NtupleMCFlightInfoTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleMCFlightInfoTool::eval - no Particle found!");
    return;
  }


  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  if (selparticles.size() != 2) {
    B2ERROR("NtupleMCFlightInfoTool::eval - you must to select exactly 2 particles in the decay (mother and daughter)!");
    return;
  }

  //get the MC MOTHER
  const MCParticle* mother = selparticles[0]->getRelatedTo<MCParticle>();

  //get the MC DAUGHTER
  const MCParticle*  daughter = selparticles[1]->getRelatedTo<MCParticle>();

  if (!mother || !daughter) {
    m_fD  = -9;
    m_fT = -9;
    return;
  }
  //mother vertex
  double mumvtxX = mother->getDecayVertex().X();
  double mumvtxY = mother->getDecayVertex().Y();
  double mumvtxZ = mother->getDecayVertex().Z();

  //daughter vertex
  double vtxX =  daughter->getDecayVertex().X();
  double vtxY =  daughter->getDecayVertex().Y();
  double vtxZ =  daughter->getDecayVertex().Z();

  // daughter MOMENTUM
  double pX = daughter->getMomentum().X();
  double pY = daughter->getMomentum().Y();
  double pZ = daughter->getMomentum().Z();
  double p = sqrt(pX * pX + pY * pY + pZ * pZ);

  //versor of the daughter momentum
  double nX = pX / p;
  double nY = pY / p;
  double nZ = pZ / p;

  //Distance between mother and daughter vertices
  double lX = vtxX - mumvtxX;
  double lY = vtxY - mumvtxY;
  double lZ = vtxZ - mumvtxZ;

  //flight distance
  m_fD  = lX * nX + lY * nY + lZ * nZ;

  //flight time
  m_fT = daughter->getMass() / Const::speedOfLight * m_fD / p;


  return;

}

