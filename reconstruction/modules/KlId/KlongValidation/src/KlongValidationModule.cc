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
//#include <mdst/dataobjects/ECLCluster.h>

// here's where the functions are hidden
#include "reconstruction/modules/KlId/KLMExpert/helperFunctions.h"

#include <TTree.h>
#include <TFile.h>
#include <TEfficiency.h>
#include <cstring>

using namespace Belle2;
using namespace std;

REG_MODULE(KlongValidation);

KlongValidationModule::KlongValidationModule(): Module()
{
  setDescription("Used to calculate validation variables for Klong efficiency validations etc...");
  addParam("outPath", m_outPath,
           "name your root file. has to end with .root",
           m_outPath);
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

  //use TEfficiency histogramms to calculate efficiency
  // name, title;xtitle;ytitle,
  m_effPhi    = new TEfficiency("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 5, -3.2, 3.2);
  m_effTheta  = new TEfficiency("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 5, 0, 3.2);
  m_effMom    = new TEfficiency("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 5, 0, 5);
  m_fakePhi   = new TEfficiency("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 5, -3.2, 3.2);
  m_fakeTheta = new TEfficiency("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 5, 0, 3.2);
  m_fakeMom   = new TEfficiency("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 5, 0, 5);


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

  // the performance of the classifier depends on the cut on the classifier
  // output. This is generally arbitrary as it depends on what the user wants.
  // Finding K_L or rejecting?
  // Therefore an arbitrary cut is chosen so that something happens and we can judge
  // if the behavior changes.
  float abitraryCut = 0.1;
  for (const KLMCluster& cluster : klmClusters) {

    if (!cluster.getRelatedTo<KlId>()) {
      B2WARNING("could not find a associated K_L Id obj. Did you run the KLM classifier?");
    }

    if (cluster.getRelatedTo<KlId>()->getKlId() > abitraryCut)
      m_reconstructedAsKl = true;
    else {
      m_reconstructedAsKl = false;
    }
    // second param is cut on
    m_isKl = KlIdHelpers::isKLMClusterSignal(cluster);

    // only fill if cluster is correctly reconstructed Kl
    if (m_reconstructedAsKl && m_isKl) {m_passed = true;}
    else {m_passed = false;}

    // used for fake rate
    if (m_reconstructedAsKl && (!m_isKl)) {m_faked = true;}
    else {m_faked = false;}

    m_phi      = cluster.getMomentum().Phi();
    m_theta    = cluster.getMomentum().Theta();
    m_momentum = std::abs(cluster.getMomentum().Mag());//Mag2(); ??

    // TEff fills 2 histogramms based on "passed" and calculates
    // the efficiency from that...

    // for the efficiency only fill something if its klong
    // so that efficiency wil be normalized to 1.
    if (m_isKl) {
      m_effPhi    -> Fill(m_passed,  m_phi);
      m_effTheta  -> Fill(m_passed,  m_theta);
      m_effMom    -> Fill(m_passed,  m_momentum);
    }

    // for the fakerate no normalisation is needed
    m_fakePhi   -> Fill(m_faked,   m_phi);
    m_fakeTheta -> Fill(m_faked,   m_theta);
    m_fakeMom   -> Fill(m_faked,   m_momentum);

  }// for klm clusters

} // event


void KlongValidationModule::terminate()
{
  // write TEff to root file ,
  m_f         -> cd();

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











