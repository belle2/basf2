/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Pablo Goldenzweig                                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleContinuumSuppressionTool.h>
#include <analysis/VariableManager/Variables.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>

using namespace Belle2;
using namespace std;

void NtupleContinuumSuppressionTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;

  if (m_strOption.empty()) {
    B2INFO("KSFW moments calculated from B primary daughters: default option");
  } else {
    if (m_strOption == "FS1") {
      B2INFO("KSFW moments calculated from B final state particles: user specified option " << m_strOption);
      m_useFS1 = true;
    } else {
      B2FATAL("Invalid option used for ContinuumSuppression ntuple tool. Set to 'FS1' to store the KSFW moments formed"
              " using the B final state particles, or leave the option empty to use the default KSFW moments formed "
              "from the B primary daughters: " << m_strOption);
    }
  }


  m_fThrustB = 0;
  m_fThrustO = 0;
  m_tree->Branch((strNames[0] + "_ThrustB").c_str(), &m_fThrustB, (strNames[0] + "_ThrustB/F").c_str());
  m_tree->Branch((strNames[0] + "_ThrustO").c_str(), &m_fThrustO, (strNames[0] + "_ThrustO/F").c_str());

  m_fCosTBTO = 0;
  m_fCosTBz = 0;
  m_tree->Branch((strNames[0] + "_CosTBTO").c_str(), &m_fCosTBTO, (strNames[0] + "_CosTBTO/F").c_str());
  m_tree->Branch((strNames[0] + "_CosTBz").c_str() , &m_fCosTBz , (strNames[0] + "_CosTBz/F").c_str());

  m_fR2 = 0;
  m_tree->Branch((strNames[0] + "_R2").c_str()     , &m_fR2     , (strNames[0] + "_R2/F").c_str());

  m_fcc1 = 0;
  m_fcc2 = 0;
  m_fcc3 = 0;
  m_fcc4 = 0;
  m_fcc5 = 0;
  m_fcc6 = 0;
  m_fcc7 = 0;
  m_fcc8 = 0;
  m_fcc9 = 0;
  m_tree->Branch((strNames[0] + "_cc1").c_str()  , &m_fcc1  , (strNames[0] + "_cc1/F").c_str());
  m_tree->Branch((strNames[0] + "_cc2").c_str()  , &m_fcc2  , (strNames[0] + "_cc2/F").c_str());
  m_tree->Branch((strNames[0] + "_cc3").c_str()  , &m_fcc3  , (strNames[0] + "_cc3/F").c_str());
  m_tree->Branch((strNames[0] + "_cc4").c_str()  , &m_fcc4  , (strNames[0] + "_cc4/F").c_str());
  m_tree->Branch((strNames[0] + "_cc5").c_str()  , &m_fcc5  , (strNames[0] + "_cc5/F").c_str());
  m_tree->Branch((strNames[0] + "_cc6").c_str()  , &m_fcc6  , (strNames[0] + "_cc6/F").c_str());
  m_tree->Branch((strNames[0] + "_cc7").c_str()  , &m_fcc7  , (strNames[0] + "_cc7/F").c_str());
  m_tree->Branch((strNames[0] + "_cc8").c_str()  , &m_fcc8  , (strNames[0] + "_cc8/F").c_str());
  m_tree->Branch((strNames[0] + "_cc9").c_str()  , &m_fcc9  , (strNames[0] + "_cc9/F").c_str());


  string FS = "";
  if (m_useFS1)
    FS = "_FS1";

  m_fmm2   = 0;
  m_fet    = 0;
  m_fhso00 = 0;
  m_fhso01 = 0;
  m_fhso02 = 0;
  m_fhso03 = 0;
  m_fhso04 = 0;
  m_fhso10 = 0;
  m_fhso12 = 0;
  m_fhso14 = 0;
  m_fhso20 = 0;
  m_fhso22 = 0;
  m_fhso24 = 0;
  m_fhoo0  = 0;
  m_fhoo1  = 0;
  m_fhoo2  = 0;
  m_fhoo3  = 0;
  m_fhoo4  = 0;
  m_tree->Branch((strNames[0] + "_mm2"   + FS).c_str(), &m_fmm2  , (strNames[0] + "_mm2"   + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_et"    + FS).c_str(), &m_fet   , (strNames[0] + "_et"    + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso00" + FS).c_str(), &m_fhso00, (strNames[0] + "_hso00" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso01" + FS).c_str(), &m_fhso01, (strNames[0] + "_hso01" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso02" + FS).c_str(), &m_fhso02, (strNames[0] + "_hso02" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso03" + FS).c_str(), &m_fhso03, (strNames[0] + "_hso03" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso04" + FS).c_str(), &m_fhso04, (strNames[0] + "_hso04" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso10" + FS).c_str(), &m_fhso10, (strNames[0] + "_hso10" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso12" + FS).c_str(), &m_fhso12, (strNames[0] + "_hso12" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso14" + FS).c_str(), &m_fhso14, (strNames[0] + "_hso14" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso20" + FS).c_str(), &m_fhso20, (strNames[0] + "_hso20" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso22" + FS).c_str(), &m_fhso22, (strNames[0] + "_hso22" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso24" + FS).c_str(), &m_fhso24, (strNames[0] + "_hso24" + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo0"  + FS).c_str(), &m_fhoo0 , (strNames[0] + "_hoo0"  + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo1"  + FS).c_str(), &m_fhoo1 , (strNames[0] + "_hoo1"  + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo2"  + FS).c_str(), &m_fhoo2 , (strNames[0] + "_hoo2"  + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo3"  + FS).c_str(), &m_fhoo3 , (strNames[0] + "_hoo3"  + FS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo4"  + FS).c_str(), &m_fhoo4 , (strNames[0] + "_hoo4"  + FS + "/F").c_str());
}

void NtupleContinuumSuppressionTool::eval(const Particle* particle)
{
  if (!particle) {
    B2ERROR("NtupleContinuumSuppressionTool::eval - no Particle found!");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  const ContinuumSuppression* qq = selparticles[0]->getRelated<ContinuumSuppression>();
  if (!qq) {
    B2ERROR("NtupleContinuumSuppressionTool::eval - no ContinuumSuppression found! Did you forget the module that builds it?");
    return;
  }
  m_fThrustB = qq->getThrustBm();
  m_fThrustO = qq->getThrustOm();
  m_fCosTBTO = qq->getCosTBTO();
  m_fCosTBz  = qq->getCosTBz();

  m_fR2  = qq->getR2();

  std::vector<float> cleoCones = qq->getCleoCones();
  m_fcc1 = cleoCones.at(0);
  m_fcc2 = cleoCones.at(1);
  m_fcc3 = cleoCones.at(2);
  m_fcc4 = cleoCones.at(3);
  m_fcc5 = cleoCones.at(4);
  m_fcc6 = cleoCones.at(5);
  m_fcc7 = cleoCones.at(6);
  m_fcc8 = cleoCones.at(7);
  m_fcc9 = cleoCones.at(8);

  // Defualt KSFW moments calculated using B primary daughters.
  std::vector<float> ksfw = qq->getKsfwFS0();

  // User speficied override: Store KSFW moments calculated using B final-state particles.
  if (m_useFS1)
    ksfw = qq->getKsfwFS1();

  m_fmm2    = ksfw.at(0);
  m_fet     = ksfw.at(1);
  m_fhso00  = ksfw.at(2);
  m_fhso01  = ksfw.at(3);
  m_fhso02  = ksfw.at(4);
  m_fhso03  = ksfw.at(5);
  m_fhso04  = ksfw.at(6);
  m_fhso10  = ksfw.at(7);
  m_fhso12  = ksfw.at(8);
  m_fhso14  = ksfw.at(9);
  m_fhso20  = ksfw.at(10);
  m_fhso22  = ksfw.at(11);
  m_fhso24  = ksfw.at(12);
  m_fhoo0   = ksfw.at(13);
  m_fhoo1   = ksfw.at(14);
  m_fhoo2   = ksfw.at(15);
  m_fhoo3   = ksfw.at(16);
  m_fhoo4   = ksfw.at(17);

}




