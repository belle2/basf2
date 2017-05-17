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
    B2INFO("KSFW moments calculated from B primary daughters, and CleoCones calculated using all final state particles: default option");
  } else {
    if ((m_strOption.find("KsfwFS1") != std::string::npos) || (m_strOption.find("CcROE") != std::string::npos)) {
      if (m_strOption.find("KsfwFS1") != std::string::npos) {
        B2INFO("KSFW moments calculated from B final state particles: user specified option " << m_strOption);
        m_useKsfwFS1 = true;
      }
      if (m_strOption.find("CcROE") != std::string::npos) {
        B2INFO("CleoCones calculated using ROE particles only: user specified option " << m_strOption);
        m_useCcROE = true;
      }
    } else {
      B2FATAL("Invalid option used for ContinuumSuppression ntuple tool. Include 'KsfwFS1' to store the KSFW moments formed "
              "using the B final state particles (otherwise the default KSFW moments using the B primary daughters will be returned). "
              "Include 'CcROE' to use CleoCones calculated using ROE particles only (otherwise the default CleoCones using all final "
              "state particles will be returned. If the option is left empty, both default KSFW and default CleoCones will be returned");
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

  string CcFS = "";
  if (m_useCcROE)
    CcFS = "_CcROE";

  m_fcc1 = 0;
  m_fcc2 = 0;
  m_fcc3 = 0;
  m_fcc4 = 0;
  m_fcc5 = 0;
  m_fcc6 = 0;
  m_fcc7 = 0;
  m_fcc8 = 0;
  m_fcc9 = 0;
  m_tree->Branch((strNames[0] + "_cc1" + CcFS).c_str()  , &m_fcc1  , (strNames[0] + "_cc1" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc2" + CcFS).c_str()  , &m_fcc2  , (strNames[0] + "_cc2" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc3" + CcFS).c_str()  , &m_fcc3  , (strNames[0] + "_cc3" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc4" + CcFS).c_str()  , &m_fcc4  , (strNames[0] + "_cc4" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc5" + CcFS).c_str()  , &m_fcc5  , (strNames[0] + "_cc5" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc6" + CcFS).c_str()  , &m_fcc6  , (strNames[0] + "_cc6" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc7" + CcFS).c_str()  , &m_fcc7  , (strNames[0] + "_cc7" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc8" + CcFS).c_str()  , &m_fcc8  , (strNames[0] + "_cc8" + CcFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_cc9" + CcFS).c_str()  , &m_fcc9  , (strNames[0] + "_cc9" + CcFS + "/F").c_str());


  string KsfwFS = "";
  if (m_useKsfwFS1)
    KsfwFS = "_KsfwFS1";

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
  m_tree->Branch((strNames[0] + "_mm2"   + KsfwFS).c_str(), &m_fmm2  , (strNames[0] + "_mm2"   + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_et"    + KsfwFS).c_str(), &m_fet   , (strNames[0] + "_et"    + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso00" + KsfwFS).c_str(), &m_fhso00, (strNames[0] + "_hso00" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso01" + KsfwFS).c_str(), &m_fhso01, (strNames[0] + "_hso01" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso02" + KsfwFS).c_str(), &m_fhso02, (strNames[0] + "_hso02" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso03" + KsfwFS).c_str(), &m_fhso03, (strNames[0] + "_hso03" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso04" + KsfwFS).c_str(), &m_fhso04, (strNames[0] + "_hso04" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso10" + KsfwFS).c_str(), &m_fhso10, (strNames[0] + "_hso10" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso12" + KsfwFS).c_str(), &m_fhso12, (strNames[0] + "_hso12" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso14" + KsfwFS).c_str(), &m_fhso14, (strNames[0] + "_hso14" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso20" + KsfwFS).c_str(), &m_fhso20, (strNames[0] + "_hso20" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso22" + KsfwFS).c_str(), &m_fhso22, (strNames[0] + "_hso22" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hso24" + KsfwFS).c_str(), &m_fhso24, (strNames[0] + "_hso24" + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo0"  + KsfwFS).c_str(), &m_fhoo0 , (strNames[0] + "_hoo0"  + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo1"  + KsfwFS).c_str(), &m_fhoo1 , (strNames[0] + "_hoo1"  + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo2"  + KsfwFS).c_str(), &m_fhoo2 , (strNames[0] + "_hoo2"  + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo3"  + KsfwFS).c_str(), &m_fhoo3 , (strNames[0] + "_hoo3"  + KsfwFS + "/F").c_str());
  m_tree->Branch((strNames[0] + "_hoo4"  + KsfwFS).c_str(), &m_fhoo4 , (strNames[0] + "_hoo4"  + KsfwFS + "/F").c_str());
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

  // Defualt CleoCones calculated using all final state particles.
  std::vector<float> cleoCones = qq->getCleoConesALL();

  // User speficied override: Use CleoCones calculated from ROE particles only.
  if (m_useCcROE)
    cleoCones = qq->getCleoConesROE();

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
  if (m_useKsfwFS1)
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




