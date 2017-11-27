/**************************************************************************
 *
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * **************************************************************************/



#include <reconstruction/modules/KlId/KlongValidation/KlongValidationModule.h>
#include <reconstruction/dataobjects/KlId.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <reconstruction/dataobjects/KlId.h>
//#include <mdst/dataobjects/ECLCluster.h>

// here's where the functions are hidden
#include "reconstruction/modules/KlId/KLMExpert/KlId.h"

#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <cstring>

using namespace Belle2;
using namespace KlId;
using namespace std;

REG_MODULE(KlongValidation);

KlongValidationModule::KlongValidationModule(): Module()
{
  setDescription("Used to calculate validation variables for Klong efficiency validations etc...");
  addParam("outPath", m_outPath,
           "name your root file. has to end with .root",
           m_outPath);
  addParam("KlId_cut", m_KlIDCut,
           "if cut < 0 then only the !trackFlag condition will be used (only neutral clusters). Otherwise the cut determines which Clusters will be used.",
           m_KlIDCut);
}



KlongValidationModule::~KlongValidationModule()
{
}


// --------------------------------------Module----------------------------------------------
void KlongValidationModule::initialize()
{
  // require existence of necessary datastore obj
  StoreArray<KLMCluster>::required();
  StoreArray<MCParticle>::required();
  //StoreArray<ECLCluster>::required();

  //StoreArray<ECLCluster> eclClusters;
  StoreArray<KLMCluster> klmClusters;


  // initialize root tree to write stuff into
  m_f =     new TFile(m_outPath.c_str(), "recreate");

  //use TH1F histogramms to calculate efficiency
  // name, title;xtitle;ytitle,
  m_effPhi_Pass      = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 50, -3.2, 3.2);
  m_Phi_all  = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 50, -3.2, 3.2);
  m_effPhi      = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 50, -3.2, 3.2);

  m_effTheta_Pass    = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 50, 0, 3.2);
  m_Theta_all  = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 50, 0, 3.2);
  m_effTheta  = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 50, 0, 3.2);

  m_effMom_Pass      = new TH1F("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 50, 0, 5);
  m_Mom_all    = new TH1F("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 50, 0, 5);
  m_effMom    = new TH1F("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 50, 0, 5);

  m_fakePhi_Pass     = new TH1F("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 50, -3.2, 3.2);
  m_fakePhi     = new TH1F("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 50, -3.2, 3.2);

  m_fakeTheta_Pass = new TH1F("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 50, 0, 3.2);
  m_fakeTheta = new TH1F("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 50, 0, 3.2);

  m_fakeMom_Pass   = new TH1F("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 50, 0, 5);
  m_fakeMom     = new TH1F("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 50, 0, 5);
}//init


void KlongValidationModule::beginRun()
{
}

void KlongValidationModule::endRun()
{
}

void KlongValidationModule::event()
{
  StoreArray<KLMCluster> klmClusters;

  for (const KLMCluster& cluster : klmClusters) {

    MCParticle* mcpart = NULL;

    if (m_KlIDCut >= 0) {
      auto klidObj = cluster.getRelatedTo<KlId>();
      if (klidObj) {
        if (klidObj->getKlId() > m_KlIDCut) {
          m_reconstructedAsKl = true;
        } else {
          m_reconstructedAsKl = false;
        }
      }
    } else {
      if (!cluster.getAssociatedTrackFlag()) {
        m_reconstructedAsKl = true;
      }
    }
    mcpart = cluster.getRelatedTo<MCParticle>();
    // find mc truth
    // go thru all mcparts mothers up to highest particle and check if its a Klong
    m_isKl = 0;
    if (mcpart == nullptr) {
      m_isKl = 0; // this is the case for beambkg
      m_isBeamBKG = 1;
    } else {
      m_isBeamBKG = 0;
      while (!(mcpart -> getMother() == nullptr)) {
        if (mcpart -> getPDG() == 130) {
          m_isKl = 1;
          break;
        }
        mcpart = mcpart -> getMother();
      }// while
    }
    // second param is cut on
    m_isKl = isKLMClusterSignal(cluster);

    // only fill if cluster is correctly reconstructed Kl
    if (m_reconstructedAsKl && m_isKl) {m_passed = true;}
    else {m_passed = false;}

    // used for fake rate
    if (m_reconstructedAsKl && (!m_isKl)) {m_faked = true;}
    else {m_faked = false;}

    m_phi      = cluster.getMomentum().Phi();
    m_theta    = cluster.getMomentum().Theta();
    m_momentum = std::abs(cluster.getMomentum().Mag());//Mag2(); ??


    if (m_passed) {
      m_effPhi_Pass    -> Fill(m_phi);
      m_effTheta_Pass  -> Fill(m_theta);
      m_effMom_Pass    -> Fill(m_momentum);
    }

    if (m_faked) {
      m_fakePhi_Pass   -> Fill(m_phi);
      m_fakeTheta_Pass -> Fill(m_theta);
      m_fakeMom_Pass   -> Fill(m_momentum);
    }

    //fil all to normalise later
    m_Phi_all -> Fill(m_phi);
    m_Theta_all -> Fill(m_theta);
    m_Mom_all -> Fill(m_momentum);

  }// for klm clusters

} // event


void KlongValidationModule::terminate()
{
  // write TEff to root file ,
  m_f         -> cd();

  // TH1F is not compatible with the validation server
  m_effPhi->Divide(m_effPhi_Pass, m_Phi_all);
  m_effTheta->Divide(m_effTheta_Pass, m_Theta_all);
  m_effMom->Divide(m_effTheta_Pass, m_Mom_all);

  m_fakePhi->Divide(m_fakePhi_Pass, m_Phi_all);
  m_fakeTheta->Divide(m_fakeTheta_Pass, m_Theta_all);
  m_fakeMom->Divide(m_fakeMom_Pass, m_Mom_all);

  // efficiencies
  m_effPhi   -> SetTitle("Klong efficiency in Phi");
  m_effPhi   -> GetListOfFunctions() -> Add(new TNamed("Description",
                                                       "Efficiency of the KLM classifier in Phi. Mainly depends on trackextrapolation in muid and KLM time resolution."));
  m_effPhi   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_effPhi   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));

  m_effTheta   -> SetTitle("Klong efficiency in Theta");
  m_effTheta   -> GetListOfFunctions() -> Add(new TNamed("Description", "Efficiency of the KLM classifier in Theta."));
  m_effTheta   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_effTheta   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));

  m_effMom   -> SetTitle("Klong efficiency in Momentum");
  m_effMom   -> GetListOfFunctions() -> Add(new TNamed("Description", "Efficiency of the KLM classifier in momentum bins."));
  m_effMom   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_effMom   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));


  // fake rates
  m_fakePhi   -> SetTitle("Klong fake rate in Phi");
  m_fakePhi   -> GetListOfFunctions() -> Add(new TNamed("Description", "Fake Rate = 1-purity of the KLM classifier in Phi."));
  m_fakePhi   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_fakePhi   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));

  m_fakeTheta   -> SetTitle("Klong fake rate in Theta");
  m_fakeTheta   -> GetListOfFunctions() -> Add(new TNamed("Description", "Fake Rate = 1-purity of the KLM classifier in Theta."));
  m_fakeTheta   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_fakeTheta   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));

  m_fakeMom   -> SetTitle("Klong fake rate in Momentum");
  m_fakeMom   -> GetListOfFunctions() -> Add(new TNamed("Description",
                                                        "Fake Rate = 1-purity of the KLM classifier in momentum bins."));
  m_fakeMom   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_fakeMom   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));

  m_effPhi    -> Write();
  m_effTheta  -> Write();
  m_effMom    -> Write();
  m_fakePhi   -> Write();
  m_fakeTheta -> Write();
  m_fakeMom   -> Write();

  m_f         -> Close();

}











