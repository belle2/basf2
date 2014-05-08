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
#include <analysis/utility/PSelectorFunctions.h>
#include <cmath>
#include <TBranch.h>
#include <TLorentzVector.h>
#include <TVector3.h>

#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/NtupleTools/NtupleROEMultiplicitiesTool.h>


void NtupleContinuumSuppressionTool::setupTree()
{
  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  if (strNames.empty()) return;
  m_fThrustB = 0;
  m_fThrustO = 0;
  m_tree->Branch((strNames[0] + "_ThrustB").c_str(), &m_fThrustB, (strNames[0] + "_ThrustB/F").c_str());
  m_tree->Branch((strNames[0] + "_ThrustO").c_str(), &m_fThrustO, (strNames[0] + "_ThrustO/F").c_str());

  m_fCosTBTO = 0;
  m_fCosTBz = 0;
  m_tree->Branch((strNames[0] + "_CosTBTO").c_str(), &m_fCosTBTO, (strNames[0] + "_CosTBTO/F").c_str());
  m_tree->Branch((strNames[0] + "_CosTBz").c_str() , &m_fCosTBz , (strNames[0] + "_CosTBz/F").c_str());

  m_k0mm2   = 0; m_k0et    = 0;
  m_k0hso00 = 0; m_k0hso01 = 0; m_k0hso02 = 0; m_k0hso03 = 0; m_k0hso04 = 0; m_k0hso10 = 0; m_k0hso12 = 0; m_k0hso14 = 0; m_k0hso20 = 0; m_k0hso22 = 0; m_k0hso24 = 0;
  m_k0hoo0  = 0; m_k0hoo1  = 0; m_k0hoo2  = 0; m_k0hoo3  = 0; m_k0hoo4  = 0;
  m_tree->Branch((strNames[0] + "_k0mm2").c_str(), &m_k0mm2  , (strNames[0] + "_k0mm2/F").c_str());
  m_tree->Branch((strNames[0] + "_k0et").c_str(), &m_k0et   , (strNames[0] + "_k0et/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso00").c_str(), &m_k0hso00, (strNames[0] + "_k0hso00/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso01").c_str(), &m_k0hso01, (strNames[0] + "_k0hso01/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso02").c_str(), &m_k0hso02, (strNames[0] + "_k0hso02/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso03").c_str(), &m_k0hso03, (strNames[0] + "_k0hso03/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso04").c_str(), &m_k0hso04, (strNames[0] + "_k0hso04/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso10").c_str(), &m_k0hso10, (strNames[0] + "_k0hso10/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso12").c_str(), &m_k0hso12, (strNames[0] + "_k0hso12/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso14").c_str(), &m_k0hso14, (strNames[0] + "_k0hso14/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso20").c_str(), &m_k0hso20, (strNames[0] + "_k0hso20/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso22").c_str(), &m_k0hso22, (strNames[0] + "_k0hso22/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hso24").c_str(), &m_k0hso24, (strNames[0] + "_k0hso24/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hoo0").c_str(), &m_k0hoo0 , (strNames[0] + "_k0hoo0/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hoo1").c_str(), &m_k0hoo1 , (strNames[0] + "_k0hoo1/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hoo2").c_str(), &m_k0hoo2 , (strNames[0] + "_k0hoo2/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hoo3").c_str(), &m_k0hoo3 , (strNames[0] + "_k0hoo3/F").c_str());
  m_tree->Branch((strNames[0] + "_k0hoo4").c_str(), &m_k0hoo4 , (strNames[0] + "_k0hoo4/F").c_str());

  m_k1mm2   = 0; m_k1et    = 0;
  m_k1hso00 = 0; m_k1hso01 = 0; m_k1hso02 = 0; m_k1hso03 = 0; m_k1hso04 = 0; m_k1hso10 = 0; m_k1hso12 = 0; m_k1hso14 = 0; m_k1hso20 = 0; m_k1hso22 = 0; m_k1hso24 = 0;
  m_k1hoo0  = 0; m_k1hoo1  = 0; m_k1hoo2  = 0; m_k1hoo3  = 0; m_k1hoo4  = 0;
  m_tree->Branch((strNames[0] + "_k1mm2").c_str(), &m_k1mm2  , (strNames[0] + "_k1mm2/F").c_str());
  m_tree->Branch((strNames[0] + "_k1et").c_str(), &m_k1et   , (strNames[0] + "_k1et/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso00").c_str(), &m_k1hso00, (strNames[0] + "_k1hso00/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso01").c_str(), &m_k1hso01, (strNames[0] + "_k1hso01/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso02").c_str(), &m_k1hso02, (strNames[0] + "_k1hso02/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso03").c_str(), &m_k1hso03, (strNames[0] + "_k1hso03/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso04").c_str(), &m_k1hso04, (strNames[0] + "_k1hso04/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso10").c_str(), &m_k1hso10, (strNames[0] + "_k1hso10/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso12").c_str(), &m_k1hso12, (strNames[0] + "_k1hso12/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso14").c_str(), &m_k1hso14, (strNames[0] + "_k1hso14/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso20").c_str(), &m_k1hso20, (strNames[0] + "_k1hso20/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso22").c_str(), &m_k1hso22, (strNames[0] + "_k1hso22/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hso24").c_str(), &m_k1hso24, (strNames[0] + "_k1hso24/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hoo0").c_str(), &m_k1hoo0 , (strNames[0] + "_k1hoo0/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hoo1").c_str(), &m_k1hoo1 , (strNames[0] + "_k1hoo1/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hoo2").c_str(), &m_k1hoo2 , (strNames[0] + "_k1hoo2/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hoo3").c_str(), &m_k1hoo3 , (strNames[0] + "_k1hoo3/F").c_str());
  m_tree->Branch((strNames[0] + "_k1hoo4").c_str(), &m_k1hoo4 , (strNames[0] + "_k1hoo4/F").c_str());
}

void NtupleContinuumSuppressionTool::eval(const Particle* particle)
{
  if (!particle) {
    printf("NtupleContinuumSuppressionTool::eval - ERROR, no Particle found!\n");
    return;
  }

  vector<const Particle*> selparticles = m_decaydescriptor.getSelectionParticles(particle);
  if (selparticles.empty()) return;

  const ContinuumSuppression* qq = DataStore::getRelated<ContinuumSuppression>(selparticles[0]);
  if (qq) {
    m_fThrustB = qq->getThrustBm();
    m_fThrustO = qq->getThrustOm();
    m_fCosTBTO = qq->getCosTBTO();
    m_fCosTBz  = qq->getCosTBz();

    // use_finalstate_for_sig == 0
    std::vector<float> ksfwFS0 = qq->getKsfwFS0();
    m_k0mm2    = ksfwFS0.at(0);
    m_k0et     = ksfwFS0.at(1);
    m_k0hso00  = ksfwFS0.at(2);
    m_k0hso01  = ksfwFS0.at(3);
    m_k0hso02  = ksfwFS0.at(4);
    m_k0hso03  = ksfwFS0.at(5);
    m_k0hso04  = ksfwFS0.at(6);
    m_k0hso10  = ksfwFS0.at(7);
    m_k0hso12  = ksfwFS0.at(8);
    m_k0hso14  = ksfwFS0.at(9);
    m_k0hso20  = ksfwFS0.at(10);
    m_k0hso22  = ksfwFS0.at(11);
    m_k0hso24  = ksfwFS0.at(12);
    m_k0hoo0   = ksfwFS0.at(13);
    m_k0hoo1   = ksfwFS0.at(14);
    m_k0hoo2   = ksfwFS0.at(15);
    m_k0hoo3   = ksfwFS0.at(16);
    m_k0hoo4   = ksfwFS0.at(17);
    // use_finalstate_for_sig == 1
    std::vector<float> ksfwFS1 = qq->getKsfwFS1();
    m_k1mm2    = ksfwFS1.at(0);
    m_k1et     = ksfwFS1.at(1);
    m_k1hso00  = ksfwFS1.at(2);
    m_k1hso01  = ksfwFS1.at(3);
    m_k1hso02  = ksfwFS1.at(4);
    m_k1hso03  = ksfwFS1.at(5);
    m_k1hso04  = ksfwFS1.at(6);
    m_k1hso10  = ksfwFS1.at(7);
    m_k1hso12  = ksfwFS1.at(8);
    m_k1hso14  = ksfwFS1.at(9);
    m_k1hso20  = ksfwFS1.at(10);
    m_k1hso22  = ksfwFS1.at(11);
    m_k1hso24  = ksfwFS1.at(12);
    m_k1hoo0   = ksfwFS1.at(13);
    m_k1hoo1   = ksfwFS1.at(14);
    m_k1hoo2   = ksfwFS1.at(15);
    m_k1hoo3   = ksfwFS1.at(16);
    m_k1hoo4   = ksfwFS1.at(17);
  }

}




