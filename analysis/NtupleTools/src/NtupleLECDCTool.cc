/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleLECDCTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/L1EmulatorVariables.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLECDCTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_nTracks = 0;
  m_nECLMatchTracks = 0;
  m_nKLMMatchTracks = 0;
  m_maxAng = 0.;
  m_tree->Branch("nTracks", &m_nTracks, "nTracks/I");
  m_tree->Branch("nECLMatchTracks", &m_nECLMatchTracks, "nECLMatchTracks/I");
  m_tree->Branch("nKLMMatchTracks", &m_nKLMMatchTracks, "nKLMMatchTracks/I");
  m_tree->Branch("maxAngtt", &m_maxAng, "maxAngtt/F");

  m_P1Bhabha = new double[5];
  m_P2Bhabha = new double[5];
  m_tree->Branch("InfTrk1", &m_P1Bhabha[0], "InfTrk1[5]/D");
  m_tree->Branch("InfTrk2", &m_P2Bhabha[0], "InfTrk2[5]/D");


}

void NtupleLECDCTool::eval(const Particle* particle)
{
// if (!particle) {
//   B2ERROR("NtupleDeltaEMbcTool::eval - no Particle found!");
//    return;
//  }
//vector<const Particle*> particle = m_decaydescriptor.getSelectionParticles(particle);
//  if (particle.empty()){cout<<"noParticle"<<endl;particle.push_back(NULL);// return;
//}
//const Particle* particle = NULL;
// Particle* particle = NULL;


  m_nTracks = Variable::nTracksLE(particle);
  m_nECLMatchTracks = Variable::nECLMatchTracksLE(particle);
  m_nKLMMatchTracks = Variable::nKLMMatchTracksLE(particle);
  m_maxAng = Variable::maxAngleTTLE(particle);
  m_P1Bhabha[0] = Variable::P1BhabhaLE(particle);
  m_P2Bhabha[0] = Variable::P2BhabhaLE(particle);
  m_P1Bhabha[1] = Variable::Theta1BhabhaLE(particle);
  m_P2Bhabha[1] = Variable::Theta2BhabhaLE(particle);
  m_P1Bhabha[2] = Variable::Phi1BhabhaLE(particle);
  m_P2Bhabha[2] = Variable::Phi2BhabhaLE(particle);
  m_P1Bhabha[3] = Variable::Charge1BhabhaLE(particle);
  m_P2Bhabha[3] = Variable::Charge2BhabhaLE(particle);
  m_P1Bhabha[4] = Variable::E1BhabhaLE(particle);
  m_P2Bhabha[4] = Variable::E2BhabhaLE(particle);

}
