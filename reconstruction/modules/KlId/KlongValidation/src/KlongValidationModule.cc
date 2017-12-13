/**************************************************************************
 *
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
#include <mdst/dataobjects/KlId.h>

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

// here's where the functions are hidden
#include "reconstruction/modules/KlId/KLMExpert/KlId.h"
#include <tracking/dataobjects/TrackClusterSeparation.h>

#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TString.h>
#include <TCut.h>
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
  m_klmClusters.isRequired();
  m_mcParticles.isRequired();

  // initialize root tree to write stuff into
  m_f =     new TFile(m_outPath.c_str(), "recreate");
  /** using TH1F because the validation server does not support TEfficiency  */
  m_effPhi_Pass      = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 5, -3.2, 3.2);
  m_Phi_all  = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 5, -3.2, 3.2);
  m_effPhi      = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 5, -3.2, 3.2);

  m_effTheta_Pass    = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 5, 0, 3.2);
  m_Theta_all  = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 5, 0, 3.2);
  m_effTheta  = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 5, 0, 3.2);

  m_effMom_Pass      = new TH1F("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 5, 0, 5);
  m_Mom_all    = new TH1F("Momentum Efficiency normalise", "Efficiency Momentum;Momentum;Count", 5, 0, 5);
  m_Mom_all_plot    = new TH1F("Momentum Efficiency all, obtained from clsuters", "Efficiency Momentum;Momentum;Count", 50, 0, 5);
  m_effMom    = new TH1F("Momentum Efficiency obtained from cluster", "Efficiency Momentum;Momentum;Efficiency", 5, 0, 5);

  m_fakePhi_Pass     = new TH1F("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 5, -3.2, 3.2);
  m_fakePhi     = new TH1F("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 5, -3.2, 3.2);

  m_fakeTheta_Pass = new TH1F("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 5, 0, 3.2);
  m_fakeTheta = new TH1F("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 5, 0, 3.2);

  m_fakeMom_Pass   = new TH1F("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 5, 0, 5);
  m_fakeMom     = new TH1F("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 5, 0, 5);

  m_time     = new TH1F("KLM Cluster Timing", "Cluster Timing;Momentum;Count", 100, -600, 600);
  m_trackSep     = new TH1F("KLM trackSeperation Distance", "KLM trackSeperation Distance;Distance;Count", 100, 0, 4000);
  m_energy     = new TH1F("KLM Energy", "KLM Energy;Energy;count", 20, 0, 5);
  m_nLayer     = new TH1F("KLM N-Layer", "N-layer;N-layer;count", 20, 0, 20);

  m_bkgPhi      = new TH1F("Phi Beam BKG", "BeamBKG #Phi;#Phi;Background count", 50, -3.2, 3.2);
  m_bkgTheta      = new TH1F("Theta Beam BKG", "BeamBKG #Theta;#Theta;Background count", 50, 0, 3.2);
  m_bkgMom      = new TH1F("Momentum Beam BKG", "BeamBKG #Momentum;#Momentum;Background count", 50, 0, 5);

  m_innermostLayer = new TH1F("Innermost layer", "Innermost layer;Innermost layer; Count", 20, 0, 20);
  m_trackFlag      = new TH1F("Track flag", "TrackFlag;Flag; Count", 2, 0, 1);
  m_ECLFlag       = new TH1F("ECL flag", "ECLFlag;Flag; Count", 2, 0, 1);

  m_klidFake       = new TH1F("Klid Fake", "klid Fake;klid; Count", m_xbins.size() - 1, m_xbins.data());
  m_klidTrue       = new TH1F("Klid True", "Klid True;klid; Count", m_xbins.size() - 1, m_xbins.data());
  m_klidAll       = new TH1F("Klid all", "klid all clusters;klid; Count", m_xbins.size() - 1, m_xbins.data());

}//initk


void KlongValidationModule::beginRun()
{
}

void KlongValidationModule::endRun()
{
}

void KlongValidationModule::event()
{

  // the performance of the classifier depends on the cut on the classifier
  // output. This is generally arbitrary as it depends on what the user wants.
  // Finding K_L or rejecting?
  // Therefore an arbitrary cut is chosen so that something happens and we can judge
  // if the behavior changes.
  for (const KLMCluster& cluster : m_klmClusters) {

    MCParticle* mcpart = NULL;

    auto klidObj = cluster.getRelatedTo<KlId>();

    if (m_KlIDCut >= 0) {
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
    m_isKl = isKLMClusterSignal(cluster, 0);

    double time = cluster.getTime();
    double nLayer = cluster.getLayers();
    double energy = cluster.getEnergy();
    double trackSep = 1.0e20;
    auto trackSeperations = cluster.getRelationsTo<TrackClusterSeparation>();
    for (auto trackSeperation :  trackSeperations) {
      double dist = trackSeperation.getDistance();
      if (dist < trackSep) {
        trackSep = dist;
      }
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

    if (m_isBeamBKG) {
      m_bkgPhi-> Fill(m_phi);
      m_bkgTheta-> Fill(m_theta);
      m_bkgMom-> Fill(m_momentum);
    }

    //fil all to normalise later
    m_Phi_all -> Fill(m_phi);
    m_Theta_all -> Fill(m_theta);
    m_Mom_all -> Fill(m_momentum);
    m_Mom_all_plot -> Fill(m_momentum);

    m_time ->Fill(time);
    m_trackSep->Fill(trackSep);
    m_energy->Fill(energy);
    m_nLayer->Fill(nLayer);
    m_innermostLayer->Fill(cluster.getInnermostLayer());
    m_trackFlag->Fill(cluster.getAssociatedTrackFlag());
    m_ECLFlag->Fill(cluster.getAssociatedEclClusterFlag());

    if (klidObj) {
      if (!m_isKl) {
        m_klidFake ->Fill(klidObj->getKlId());
      }
      if (m_isKl) {
        m_klidTrue ->Fill(klidObj->getKlId());
      }
      m_klidAll ->Fill(klidObj->getKlId());
    }
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

  std::vector<double> efficiency(m_xbins.size());
  std::vector<double> purity(m_xbins.size());
  std::vector<double> backrej(m_xbins.size());
  double NallKlong = m_klidTrue->GetEntries();
  double NallFakes = m_klidFake->GetEntries();
  for (unsigned int i = 0; i < m_xbins.size() - 1; ++i) {
    double NtruePositive = m_klidTrue->Integral(i, m_xbins.size() - 1);
    double NfalsePositive = m_klidFake->Integral(i, m_xbins.size() - 1);
    if ((NtruePositive + NfalsePositive) <= 0) {
      purity[i] = 0;
    } else {
      purity[i] = NtruePositive / (NtruePositive + NfalsePositive);
    }
    if (NallFakes) {
      backrej[i] = 1 - NfalsePositive / NallFakes;
    } else {
      backrej[i] = 0;
    }
    if (NallKlong <= 0) {
      B2WARNING("No Klongs found in ROC calculation.");
    } else {
      efficiency[i] = NtruePositive / NallKlong;
    }
  }

  m_ROC = new TGraph(m_xbins.size(), efficiency.data(), purity.data());
  m_backRej = new TGraph(m_xbins.size(), efficiency.data(), backrej.data());

  m_ROC->SetMarkerStyle(3);
  m_backRej->SetMarkerStyle(3);
  m_ROC->SetLineColorAlpha(kBlue, 0.0);
  m_backRej->SetLineColorAlpha(kBlue, 0.0);
  m_ROC->GetXaxis()->SetTitle("Efficiency");
  m_backRej->GetXaxis()->SetTitle("Efficiency");
  m_ROC->GetYaxis()->SetTitle("Purity");
  m_backRej->GetYaxis()->SetTitle("Background rejection");
  m_ROC-> SetTitle("Klong Purity vs Efficiency");
  m_backRej-> SetTitle("Klong background rejection vs Efficiency");
  m_ROC-> GetListOfFunctions() -> Add(new TNamed("Description",
                                                 "Purity vs Efficiency each point represents a cut on the klong ID."));
  m_ROC   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_backRej   -> GetListOfFunctions() -> Add(new TNamed("Check", "Should be as high as possible"));
  m_ROC   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));
  m_backRej   -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));

  std::vector<std::string> titles = {"K_L ID", "All Momenta generated", "Klong efficiency in Phi", "Klong efficiency in Theta", "Klong efficiency in Momentum", "Klong fake rate in Phi", "Klong fake rate in Theta", "Klong fake rate in Momentum", "KLM time", "KLM distance to next track", "KLM energy", "KLM nLayer", "Beam BKG in Phi", "Beam BKG in Theta", "Beam BKG in Momentum", "Innermost layer", "Track flag", "ECL Flag" };

  std::vector<TH1F*> histograms = {m_klidAll, m_Mom_all_plot, m_effPhi, m_effTheta,
                                   m_effMom, m_fakePhi, m_fakeTheta, m_fakeMom,
                                   m_time, m_trackSep, m_energy, m_nLayer, m_bkgPhi,
                                   m_bkgTheta, m_bkgMom, m_innermostLayer, m_trackFlag, m_ECLFlag
                                  };
  unsigned int counter = 0;
  for (auto hist  : histograms) {
    hist -> SetTitle(titles[counter].c_str());
    hist -> GetListOfFunctions() -> Add(new TNamed("Description", titles[counter]));
    hist -> GetListOfFunctions() -> Add(new TNamed("Check", "Should not change"));
    hist -> GetListOfFunctions() -> Add(new TNamed("Contact", "jkrohn@student.unimelb.edu.au"));
    hist -> Write();
    ++counter;
  }
  //this guy is a tgraph
  m_ROC       -> Write();
  m_backRej   -> Write();
  m_f         -> Close();
}

