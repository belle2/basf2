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
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
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
  m_E12 = 0.;
  m_AngleGG = -1.;
  m_AngleGT = -1.;
//  m_EC12CMS=-1.;

//m_tree->Branch("nECLClusters", &m_nECLClusters, "nECLClusters/I");
  m_tree->Branch("nECLCluters", &m_ICN, "nECLClusters/I");
  m_tree->Branch("Etot", &m_Etot, "Etot/F");
  m_tree->Branch("E12", &m_E12, "E12/F");
  m_tree->Branch("AngleGG", &m_AngleGG, "AngleGG/F");
  m_tree->Branch("maxAngGT", &m_AngleGT, "AngleGT/F");
//  m_tree->Branch("EC12CMS", &m_EC12CMS, "EC12CMS/F");


// m_RingSum     = new float[17];
//  m_BhabhaType = new float[18];
  m_Vp4E1 = new float[3];
  m_Vp4E2 = new float[3];
  m_Vp4Neutral = new float[3];
  m_ECMS = new float[4];
  m_AngCT = new float[4];
  m_AngGT = new float[2];

  m_tree->Branch("InfCluster1",  &m_Vp4E1[0], "InfCluter1[3]/F");
  m_tree->Branch("InfCluster2",  &m_Vp4E2[0], "InfCluster2[3]/F");
  m_tree->Branch("InfNeutralCluster",  &m_Vp4Neutral[0], "InfNeutralCluster[3]/F");
  m_tree->Branch("ECMS",  &m_ECMS[0], "ECMS[4]/F");
  m_tree->Branch("AngCT",  &m_AngCT[0], "AngCT[4]/F");
  m_tree->Branch("AngGT",  &m_AngGT[0], "AngGT[2]/F");

}

void NtupleLEECLTool::eval(const Particle* particle)
{

  m_ICN = Variable::nClustersLE(particle);
  m_Etot = Variable::EtotLE(particle);
  m_E12 = Variable::EC12LE(particle);

  m_Vp4E1[0] = Variable::EC1LE(particle);
  m_Vp4E1[1] = Variable::ThetaC1LE(particle);
  m_Vp4E1[2] = Variable::PhiC1LE(particle);
  //m_Vp4E1[4] = Variable::EC1CMSLE(particle);

  m_Vp4E2[0] = Variable::EC2LE(particle);
  m_Vp4E2[1] = Variable::ThetaC2LE(particle);
  m_Vp4E2[2] = Variable::PhiC2LE(particle);
  //m_Vp4E2[4] = Variable::EC2CMSLE(particle);

  m_Vp4Neutral[0] = Variable::ENeutralLE(particle);
  m_Vp4Neutral[1] = Variable::ThetaNeutralLE(particle);
  m_Vp4Neutral[2] = Variable::PhiNeutralLE(particle);

  m_ECMS[0] = Variable::EC1CMSLE(particle);
  m_ECMS[1] = Variable::EC2CMSLE(particle);
  m_ECMS[2] = Variable::EC12CMSLE(particle);
  m_ECMS[3] = Variable::ENeutralCMSLE(particle);

  m_AngleGG = Variable::AngleGGLE(particle);
  m_AngleGT = Variable::AngleGTLE(particle);
  m_AngCT[0] = Variable::AngleT1C1LE(particle);
  m_AngCT[1] = Variable::AngleT1C2LE(particle);
  m_AngCT[2] = Variable::AngleT2C1LE(particle);
  m_AngCT[3] = Variable::AngleT2C2LE(particle);

  m_AngGT[0] = Variable::AngleG1T1LE(particle);
  m_AngGT[1] = Variable::AngleG1T2LE(particle);
}
