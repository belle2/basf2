/**************************************************************************
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
#include <framework/gearbox/Const.h>

// here's where the functions are hidden
#include <reconstruction/modules/KlId/KLMExpert/KlId.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>

#include <TFile.h>
#include <TGraph.h>
#include <cstring>

using namespace std;
using namespace Belle2;
using namespace Belle2::KlongId;

REG_MODULE(KlongValidation);

KlongValidationModule::KlongValidationModule(): Module()
{
  setDescription("Module used by the validation server to generate root files for the K0L validation. It calculates fake rates and efficiencies.");
  addParam("outputName", m_outputName,
           "Name of the output root file (must end with .root)",
           m_outputName);
  addParam("KlIdCut", m_KlIdCut,
           "If cut < 0, then only the neutral KLMClusters will be used. Otherwise, only the KLMClusters that satisfies the selection will be used.",
           m_KlIdCut);
}

KlongValidationModule::~KlongValidationModule()
{
}

void KlongValidationModule::initialize()
{
  // require existence of necessary datastore obj
  m_klmClusters.isRequired();
  m_mcParticles.isRequired();

  // initialize root tree to write stuff into
  m_f =     new TFile(m_outputName.c_str(), "recreate");
  /** using TH1F because the validation server does not support TEfficiency  */
  m_Phi_Pass      = new TH1F("Phi Passed", "Passed #Phi;#Phi;Count", 32, -3.2, 3.2);
  m_Phi_all  = new TH1F("Phi All", "All #Phi;#Phi;Count", 32, -3.2, 3.2);
  m_effPhi      = new TH1F("Phi Efficiency", "Efficiency #Phi;#Phi;Efficiency", 32, -3.2, 3.2);

  m_Theta_Pass    = new TH1F("Theta Passed", "Passed #Theta;#Theta;Count", 32, 0, 3.2);
  m_Theta_all  = new TH1F("Theta All", "All #Theta;#Theta;Count", 32, 0, 3.2);
  m_effTheta  = new TH1F("Theta Efficiency", "Efficiency #Theta;#Theta;Efficiency", 32, 0, 3.2);

  m_Mom_Pass      = new TH1F("Momentum Efficiency", "Efficiency Momentum;Momentum;Efficiency", 25, 0, 5);
  m_Mom_all    = new TH1F("Momentum Efficiency normalise", "Efficiency Momentum;Momentum;Count", 25, 0, 5);
  m_Mom_all_plot    = new TH1F("Momentum Efficiency all, obtained from clusters", "Efficiency Momentum;Momentum;Count", 25, 0, 5);
  m_effMom    = new TH1F("Momentum Efficiency obtained from cluster", "Efficiency Momentum;Momentum;Efficiency", 25, 0, 5);

  m_fakePhi_Pass     = new TH1F("Phi Fake Passsed", "Fake Passed #Phi;#Phi;Count", 32, -3.2, 3.2);
  m_fakePhi     = new TH1F("Phi Fake Rate", "Fake Rate #Phi;#Phi;Fake Rate", 32, -3.2, 3.2);

  m_fakeTheta_Pass = new TH1F("Theta Fake Passed", "Fake Passed #Theta;#Theta;Count", 32, 0, 3.2);
  m_fakeTheta = new TH1F("Theta Fake Rate", "Fake Rate #Theta;#Theta;Fake Rate", 32, 0, 3.2);

  m_fakeMom_Pass   = new TH1F("Momentum Fake Passed", "Momentum Fake Passed;Momentum;Count", 25, 0, 5);
  m_fakeMom     = new TH1F("Momentum Fake Rate", "Momentum Fake Rate;Momentum;Fake Rate", 25, 0, 5);

  m_time     = new TH1F("KLM Cluster Time", "Cluster Timing;Cluster time;Count", 125, -5, 20);
  m_trackSep     = new TH1F("KLM trackSeperation Distance", "KLM trackSeperation Distance;Distance;Count", 100, 0, 4000);
  m_energy     = new TH1F("KLM Energy", "KLM Energy;Energy;count", 25, 0, 5);
  m_nLayer     = new TH1F("KLM N-Layer", "N-layer;N-layer;count", 20, 0, 20);

  m_bkgPhi      = new TH1F("Phi Beam BKG", "BeamBKG #Phi;#Phi;Background count", 32, -3.2, 3.2);
  m_bkgTheta      = new TH1F("Theta Beam BKG", "BeamBKG #Theta;#Theta;Background count", 32, 0, 3.2);
  m_bkgMom      = new TH1F("Momentum Beam BKG", "BeamBKG #Momentum;#Momentum;Background count", 25, 0, 5);

  m_innermostLayer = new TH1F("Innermost layer", "Innermost layer;Innermost layer; Count", 20, 0, 20);
  m_trackFlag      = new TH1F("Track flag", "TrackFlag;Flag; Count", 2, 0, 1);
  m_ECLFlag       = new TH1F("ECL flag", "ECLFlag;Flag; Count", 2, 0, 1);

  m_klidFake       = new TH1F("Klid Fake", "klid Fake;klid; Count", m_xbins.size() - 1, m_xbins.data());
  m_klidTrue       = new TH1F("Klid True", "Klid True;klid; Count", m_xbins.size() - 1, m_xbins.data());
  m_klidAll       = new TH1F("Klid all", "klid all clusters;klid; Count", m_xbins.size() - 1, m_xbins.data());
}

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

    if (m_KlIdCut >= 0) {
      if (klidObj) {
        if (klidObj->getKlId() > m_KlIdCut) {
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
        if (mcpart -> getPDG() == Const::Klong.getPDGCode()) {
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
    m_momentum = std::abs(cluster.getMomentumMag());

    if (m_passed) {
      m_Phi_Pass    -> Fill(m_phi);
      m_Theta_Pass  -> Fill(m_theta);
      m_Mom_Pass    -> Fill(m_momentum);
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

    m_klidAll->SetMinimum(0.);
    m_Mom_all_plot->SetMinimum(0.);
    m_effPhi->SetMinimum(0.);
    m_effTheta->SetMinimum(0.);
    m_effMom->SetMinimum(0.);
    m_fakePhi->SetMinimum(0.);
    m_fakeTheta->SetMinimum(0.);
    m_fakeMom->SetMinimum(0.);
    m_time->SetMinimum(0.);
    m_trackSep->SetMinimum(0.);
    m_energy->SetMinimum(0.);
    m_nLayer->SetMinimum(0.);
    m_bkgPhi->SetMinimum(0.);
    m_bkgTheta->SetMinimum(0.);
    m_bkgMom->SetMinimum(0.);
    m_innermostLayer->SetMinimum(0.);
    m_trackFlag->SetMinimum(0.);
    m_ECLFlag->SetMinimum(0.);

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

}

void KlongValidationModule::terminate()
{
  // write TEff to root file ,
  m_f         -> cd();
  // TH1F is not compatible with the validation server
  m_effPhi->Divide(m_Phi_Pass, m_Phi_all);
  m_effTheta->Divide(m_Theta_Pass, m_Theta_all);
  m_effMom->Divide(m_Mom_Pass, m_Mom_all);

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
  m_ROC   -> GetListOfFunctions() -> Add(new TNamed("Contact", "fnc@lnf.infn.it"));
  m_backRej   -> GetListOfFunctions() -> Add(new TNamed("Contact", "fnc@lnf.infn.it"));

  // tuple: pointer to the plot, name of the plot, true for shifter plots
  std::vector<std::tuple<TH1F*, std::string, bool>> histograms;
  histograms.push_back(make_tuple(m_klidAll, "KlId distribution", true));
  histograms.push_back(make_tuple(m_Mom_all_plot, "All Momenta generated", false));
  histograms.push_back(make_tuple(m_effPhi, "KlId efficiency in Phi", false));
  histograms.push_back(make_tuple(m_effTheta, "KlId efficiency in Theta", true));
  histograms.push_back(make_tuple(m_effMom, "KlId efficiency in Momentum", true));
  histograms.push_back(make_tuple(m_fakePhi, "KlId fake rate in Phi", false));
  histograms.push_back(make_tuple(m_fakeTheta, "KlId fake rate in Theta", true));
  histograms.push_back(make_tuple(m_fakeMom, "KlId fake rate in Momentum", true));
  histograms.push_back(make_tuple(m_time, "KLMClusters time", true));
  histograms.push_back(make_tuple(m_trackSep, "Distance of KLMCluster to closest Track", true));
  histograms.push_back(make_tuple(m_energy, "KLMClusters energy", false));
  histograms.push_back(make_tuple(m_nLayer, "KLMClusters nLayer", false));
  histograms.push_back(make_tuple(m_innermostLayer, "KLMClusters innermostLayer", false));
  histograms.push_back(make_tuple(m_bkgPhi, "Beam bkg in Phi", false));
  histograms.push_back(make_tuple(m_bkgTheta, "Beam bkg in Theta", false));
  histograms.push_back(make_tuple(m_bkgMom, "Beam bkg in Momentum", false));
  histograms.push_back(make_tuple(m_trackFlag, "Track flag", false));
  histograms.push_back(make_tuple(m_ECLFlag, "ECLCluster flag", false));

  for (auto hist  : histograms) {
    std::get<0>(hist) -> SetTitle(std::get<1>(hist).c_str());
    std::get<0>(hist) -> GetListOfFunctions() -> Add(new TNamed("Description", std::get<1>(hist)));
    std::get<0>(hist) -> GetListOfFunctions() -> Add(new TNamed("Check", "Should not change"));
    std::get<0>(hist) -> GetListOfFunctions() -> Add(new TNamed("Contact", "fnc@lnf.infn.it"));
    if (std::get<2>(hist))
      std::get<0>(hist) -> GetListOfFunctions() -> Add(new TNamed("MetaOptions", "shifter,pvalue-warn=0.99,pvalue-error=0.02"));
    else
      std::get<0>(hist) -> GetListOfFunctions() -> Add(new TNamed("MetaOptions", "pvalue-warn=0.99,pvalue-error=0.1"));
    std::get<0>(hist) -> Write();
  }

  //this guy is a tgraph
  m_ROC       -> Write();
  m_backRej   -> Write();
  m_f         -> Close();
}
