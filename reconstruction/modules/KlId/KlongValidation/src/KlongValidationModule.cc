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

#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
//#include <mdst/dataobjects/ECLCluster.h>

#include <tracking/trackFindingCDC/tmva/Recorder.h>

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
  m_effPhi    = new TEfficiency("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 50, -3.2, 3.2);

  m_effTheta  = new TEfficiency("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 50, 0, 3.2);
  m_effMom    = new TEfficiency("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 50, 0, 5);
  m_fakePhi   = new TEfficiency("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 50, -3.2, 3.2);
  m_fakeTheta = new TEfficiency("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 50, 0, 3.2);
  m_fakeMom   = new TEfficiency("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 50, 0, 5);


}//init


void KlongValidationModule::beginRun()
{
}

void KlongValidationModule::endRun()
{
}

void KlongValidationModule::event()
{
  // objects needed
  StoreArray<KLMCluster> klmClusters;
  //StoreArray<ECLCluster> eclClusters;

  // ------------------ KLM CLUSTERS

  //TODO in future: - add ecl clusters
  //                - add classification
  //                - two cuts?
  //
  for (const KLMCluster& cluster : klmClusters) {

    MCParticle* mcpart = NULL;


    // cluster reconstructed as klong?
    if (!cluster.getAssociatedTrackFlag()) {m_reconstructedAsKl = true;}

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
  m_effPhi   -> GetListOfFunctions() -> Add(new TNamed("Description", m_effPhi -> GetTitle()));
  m_effPhi   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));

  m_effTheta   -> SetTitle("Klong efficiency in Theta");
  m_effTheta   -> GetListOfFunctions() -> Add(new TNamed("Description", m_effTheta -> GetTitle()));
  m_effTheta   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));

  m_effMom   -> SetTitle("Klong efficiency in Momentum");
  m_effMom   -> GetListOfFunctions() -> Add(new TNamed("Description", m_effMom -> GetTitle()));
  m_effMom   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));


  // fake rates
  m_fakePhi   -> SetTitle("Klong fake rate in Phi");
  m_fakePhi   -> GetListOfFunctions() -> Add(new TNamed("Description", m_fakePhi -> GetTitle()));
  m_fakePhi   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));

  m_fakeTheta   -> SetTitle("Klong dake rate in Theta");
  m_fakeTheta   -> GetListOfFunctions() -> Add(new TNamed("Description", m_fakeTheta -> GetTitle()));
  m_fakeTheta   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));

  m_fakeMom   -> SetTitle("Klong fake rate in Momentum");
  m_fakeMom   -> GetListOfFunctions() -> Add(new TNamed("Description", m_fakeMom -> GetTitle()));
  m_fakeMom   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));


  m_effPhi    -> Write();
  m_effTheta  -> Write();
  m_effMom    -> Write();
  m_fakePhi   -> Write();
  m_fakeTheta -> Write();
  m_fakeMom   -> Write();

  m_f         -> Close();

}











