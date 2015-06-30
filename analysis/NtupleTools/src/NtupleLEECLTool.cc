/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleLEECLTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/L1EmulatorVariables.h>
#include <skim/L1Emulator/dataobjects/L1EmulationInformation.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLEECLTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
//  m_nECLClusters=0;
  m_ICN = 0;
  m_Etot = 0.;
  m_AngleGG = -1.;

//m_tree->Branch("nECLClusters", &m_nECLClusters, "nECLClusters/I");
  m_tree->Branch("nECLCluters", &m_ICN, "nECLClusters/I");
  m_tree->Branch("Etot", &m_Etot, "Etot/F");
  m_tree->Branch("AngleGG", &m_AngleGG, "AngleGG/F");


// m_RingSum     = new float[17];
//  m_BhabhaType = new float[18];
  m_Vp4E1 = new float[4];
  m_Vp4E2 = new float[4];
  //for (int i= 0; i< 17; i++)
//   m_tree->Branch("RingSum",  &m_RingSum[0], "RingSum[17]/F");
  //for (int j = 0; j < 18; j++)
//   m_tree->Branch("BhabhaType",  &m_BhabhaType[0],"BhabhaType[18]/F");
  m_tree->Branch("InfCluster1",  &m_Vp4E1[0], "InfCluter1[4]/F");
  m_tree->Branch("InfCluster2",  &m_Vp4E2[0], "InfCluster2[4]/F");

}

void NtupleLEECLTool::eval(const Particle* particle)
{
// if (!particle) {
//   B2ERROR("NtupleDeltaEMbcTool::eval - no Particle found!");
//    return;
//  }
//vector<const Particle*> particle = m_decaydescriptor.getSelectionParticles(particle);
//  if (particle.empty()){cout<<"noParticle"<<endl;particle.push_back(NULL);// return;
//}

  m_ICN = Variable::nClustersLE(particle);
  m_Etot = Variable::EtotLE(particle);
//m_ECLtoGDL    =LEecl::getECLtoGDL();
//L1EmulationInformation* LEInfo= LEInfos[0];
//std::vector<float> phisum=LEInfos[0]->getRingSum();
//std::vector<float> bhabhatype=LEInfos[0]->getBhabhaType();
// for(int i=0;i<17;i++)m_RingSum[i]=phisum[i];
// for(int i=0;i<18;i++)m_BhabhaType[i]=bhabhatype[i];

  m_Vp4E1[0] = Variable::EC1LE(particle);
  m_Vp4E1[1] = Variable::RC1LE(particle);
  m_Vp4E1[2] = Variable::ThetaC1LE(particle);
  m_Vp4E1[3] = Variable::PhiC1LE(particle);


  m_Vp4E2[0] = Variable::EC2LE(particle);
  m_Vp4E2[1] = Variable::RC2LE(particle);
  m_Vp4E2[2] = Variable::ThetaC2LE(particle);
  m_Vp4E2[3] = Variable::PhiC2LE(particle);

  m_AngleGG = Variable::AngleGGLE(particle);
}
