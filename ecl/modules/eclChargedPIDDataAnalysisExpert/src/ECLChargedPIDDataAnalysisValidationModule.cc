#include <ecl/modules/eclChargedPIDDataAnalysisExpert/ECLChargedPIDDataAnalysisValidationModule.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ECLChargedPIDDataAnalysisValidation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLChargedPIDDataAnalysisValidationModule::ECLChargedPIDDataAnalysisValidationModule() : Module()
{
  // Set module properties
  setDescription("This module dumps a set of histograms with ECL charged PID-related info used for validation.");

  // Default charged stable pdgIds (particles & antiparticles)
  std::vector<int> defaultChargedPdgIds;
  for (const auto& hypo : Const::chargedStableSet) {
    defaultChargedPdgIds.push_back(hypo.getPDGCode());
    defaultChargedPdgIds.push_back(-hypo.getPDGCode());
  }

  addParam("inputPdgIdList", m_inputPdgIdList,
           "The list of (signed) pdgIds of the charged stable particles of interest. Default is ALL charged stable particles.",
           defaultChargedPdgIds);
  addParam("outputFileName", m_outputFileName,
           "The base name of the output file. The pdgId of the charged particle is appended to the name.",
           std::string("ECLChargedPid"));

}

ECLChargedPIDDataAnalysisValidationModule::~ECLChargedPIDDataAnalysisValidationModule()
{
}


void ECLChargedPIDDataAnalysisValidationModule::initialize()
{
  B2INFO("Initialising ROOT objects...");

  // Convert pdgId list to a set to remove any accidental repetitions.
  m_inputPdgIdSet = std::set<int>(m_inputPdgIdList.begin(), m_inputPdgIdList.end());

  std::string chargedPdgIdStr;
  std::string fname;

  for (const auto& chargedPdgId : m_inputPdgIdSet) {

    // Check if this pdgId is that of a legit Const::ChargedStable particle.
    if (!isValidChargedPdg(std::abs(chargedPdgId))) {
      B2FATAL("PDG: " << chargedPdgId << " in m_inputPdgIdSet is not that of a valid particle in Const::chargedStableSet! Aborting...");
    }

    // Get the idx of this pdgId in the Const::chargedStableSet
    auto chargedIdx = Const::chargedStableSet.find(std::abs(chargedPdgId)).getIndex();

    if (chargedPdgId > 0) {
      chargedPdgIdStr = std::to_string(chargedPdgId);
    } else {
      chargedPdgIdStr = "anti" + std::to_string(std::abs(chargedPdgId));
      // Add offset to idx.
      chargedIdx += Const::ChargedStable::c_SetSize;
    }

    fname = m_outputFileName + "_" + chargedPdgIdStr + ".root";

    m_outputFile[chargedIdx] = new TFile(fname.c_str(), "RECREATE");

    m_tree[chargedIdx] = new TTree("ECLChargedPid", "ECLChargedPid");
    m_tree[chargedIdx]->Branch("p", &m_p[chargedIdx], "p/F");
    m_tree[chargedIdx]->Branch("trkTheta", &m_trkTheta[chargedIdx], "trkTheta/F");
    m_tree[chargedIdx]->Branch("trkPhi", &m_trkPhi[chargedIdx], "trkPhi/F");
    m_tree[chargedIdx]->Branch("clusterTheta", &m_clusterTheta[chargedIdx], "clusterTheta/F");
    m_tree[chargedIdx]->Branch("clusterPhi", &m_clusterPhi[chargedIdx], "clusterPhi/F");
    m_tree[chargedIdx]->Branch("trackClusterMatch", &m_trackClusterMatch[chargedIdx], "trackClusterMatch/B");
    m_tree[chargedIdx]->Branch("logl_sig", &m_logl_sig[chargedIdx], "logl_sig/F");
    m_tree[chargedIdx]->Branch("logl_bkg", &m_logl_bkg[chargedIdx], "logl_bkg/F");
    m_tree[chargedIdx]->Branch("deltalogl", &m_deltalogl[chargedIdx], "deltalogl/F");
    m_tree[chargedIdx]->Branch("pid_glob", &m_pid[chargedIdx], "pid_glob/F");

  }
}

void ECLChargedPIDDataAnalysisValidationModule::beginRun()
{
}

void ECLChargedPIDDataAnalysisValidationModule::event()
{

  std::string chargedPdgIdStr;

  for (const auto& chargedPdgId : m_inputPdgIdSet) {

    // Get the idx of this pdgId in the Const::chargedStableSet
    auto chargedIdx = Const::chargedStableSet.find(std::abs(chargedPdgId)).getIndex();

    if (chargedPdgId < 0) {
      // Add offset to idx for antiparticles.
      chargedIdx += Const::ChargedStable::c_SetSize;
    }

    // Initialise branches to unphysical values.
    m_p[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_trkTheta[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_trkPhi[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_clusterTheta[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_clusterPhi[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_trackClusterMatch[chargedIdx] = std::numeric_limits<char>::quiet_NaN();
    m_logl_sig[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_logl_bkg[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_deltalogl[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_pid[chargedIdx] = std::numeric_limits<float>::quiet_NaN();

    StoreArray<MCParticle> particles;

    for (const auto& particle : particles) {

      if (!particle.hasStatus(MCParticle::c_PrimaryParticle)) continue; // Only check primaries.
      if (particle.hasStatus(MCParticle::c_Initial)) continue; // Ignore initial particles.
      if (particle.hasStatus(MCParticle::c_IsVirtual)) continue; // Ignore virtual particles.

      // Skip all particles expect for the one of interest.
      if (particle.getPDG() != chargedPdgId) continue;

      // Get the matching track w/ max momentum.
      int itrack(0);
      int itrack_max(-1);
      double p_max(-999.0);
      for (const auto& track : particle.getRelationsFrom<Track>()) {
        const auto fitRes = track.getTrackFitResultWithClosestMass(Const::pion);
        if (!fitRes) continue;
        if (fitRes->getMomentum().Mag() > p_max) {
          p_max = fitRes->getMomentum().Mag();
          itrack_max = itrack;
        }
        itrack++;
      }
      if (itrack_max < 0) continue; // Go to next particle if no track found.

      const auto track = particle.getRelationsFrom<Track>()[itrack_max];
      const auto fitRes = track->getTrackFitResultWithClosestMass(Const::pion);

      m_p[chargedIdx] = p_max;
      m_trkTheta[chargedIdx] = fitRes->get4Momentum().Theta();
      m_trkPhi[chargedIdx] = fitRes->get4Momentum().Phi();

      // Get the index of the ECL cluster matching this track.
      int icluster_match(-1);
      auto eclClusters = track->getRelationsTo<ECLCluster>();
      for (unsigned int icluster(0); icluster < eclClusters.size(); ++icluster) {
        const auto eclCluster = eclClusters[icluster];
        if (!eclCluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) continue;
        if (!eclCluster->isTrack()) continue;
        icluster_match = icluster;
        break;
      }
      // If no cluster match, skip to next particle, but keep track of counter.
      if (icluster_match < 0) {
        m_trackClusterMatch[chargedIdx] = 0;
        continue;
      }

      const auto eclCluster = eclClusters[icluster_match];

      m_clusterTheta[chargedIdx] = eclCluster->getTheta();
      m_clusterPhi[chargedIdx] = eclCluster->getPhi();
      m_trackClusterMatch[chargedIdx] = 1;

      const auto eclLikelihood = track->getRelated<ECLPidLikelihood>();

      // The signal likelihood corresponds to the input chargedPdgId.
      const auto chargedStableSig = Const::chargedStableSet.find(std::abs(chargedPdgId));
      // For deltaLogL, we do a binary comparison sig/bkg.
      // If sig=pion, use bkg=kaon. Otherwise, bkg=pion.
      const auto chargedStableBkg = (chargedStableSig == Const::pion) ? Const::kaon : Const::pion;

      double lh_sig = eclLikelihood->getLikelihood(chargedStableSig);
      double lh_bkg = eclLikelihood->getLikelihood(chargedStableBkg);

      m_logl_sig[chargedIdx] = log(lh_sig);
      m_logl_bkg[chargedIdx] = log(lh_bkg);
      m_deltalogl[chargedIdx] = log(lh_bkg) - log(lh_sig);

      // Get the global likelihood.
      double lh_all(0);
      for (const auto& chargedStable : Const::chargedStableSet) {
        lh_all += eclLikelihood->getLikelihood(chargedStable);
      }

      m_pid[chargedIdx] = lh_sig / lh_all;

    }

    m_tree[chargedIdx]->Fill();
  }
}

void ECLChargedPIDDataAnalysisValidationModule::endRun()
{
}

void ECLChargedPIDDataAnalysisValidationModule::terminate()
{

  std::string chargedPdgIdStr;

  for (const auto& chargedPdgId : m_inputPdgIdSet) {

    // Get the idx of this pdgId in the Const::chargedStableSet
    auto chargedIdx = Const::chargedStableSet.find(std::abs(chargedPdgId)).getIndex();

    if (chargedPdgId > 0) {
      chargedPdgIdStr = std::to_string(chargedPdgId);
    } else {
      chargedPdgIdStr = "anti" + std::to_string(std::abs(chargedPdgId));
      // Add offset to idx for antiparticles.
      chargedIdx += Const::ChargedStable::c_SetSize;
    }

    m_outputFile[chargedIdx]->cd();

    // Add summary description of validation file content.
    std::string fileDescription = "ECL Charged PID control plots for charged stable particle - Signal PDG = " + chargedPdgIdStr;
    TNamed("Description", fileDescription.c_str()).Write();

    dumpPIDVars(m_tree[chargedIdx], chargedPdgIdStr);
    dumpPIDEfficiency(m_tree[chargedIdx], chargedPdgIdStr);
    dumpTrkClusMatchingEfficiency(m_tree[chargedIdx], chargedPdgIdStr);

    m_tree[chargedIdx]->Write();

    m_outputFile[chargedIdx]->Close();

  }
}

void ECLChargedPIDDataAnalysisValidationModule::dumpPIDVars(TTree* tree, const std::string& pdgIdStr)
{

  // Histogram of global PID distribution for signal=pdgIdStr.
  std::string h_pid_name = "h_pid_" + pdgIdStr;
  TH1F* h_pid = new TH1F(h_pid_name.c_str(), h_pid_name.c_str(), 50, -0.5, 1.2);

  // Histogram of deltalogl(bkg=(pdgIdStr!=pion)?pion:kaon, sig=pdgIdStr).
  std::string h_deltalogl_name = "h_deltalogl_" + pdgIdStr;
  double deltalogl_min = -20.0;
  double deltalogl_max = 20.0;
  TH1F* h_deltalogl = new TH1F(h_deltalogl_name.c_str(), h_deltalogl_name.c_str(), 40, deltalogl_min, deltalogl_max);

  tree->Project(h_pid_name.c_str(), "pid_glob");
  tree->Project(h_deltalogl_name.c_str(), "deltalogl");

  // Make sure the plots show the u/oflow.
  paintUnderOverflow(h_pid);
  paintUnderOverflow(h_deltalogl);

  h_pid->SetOption("HIST");
  h_deltalogl->SetOption("HIST");

  // Add histogram info.
  h_pid->GetListOfFunctions()->Add(new TNamed("Description", TString::Format("ECL global PID distribution - Signal PDG = %s",
                                              pdgIdStr.c_str()).Data()));
  h_pid->GetListOfFunctions()->Add(new TNamed("Check",
                                              "The more peaked at 1, the better. Non-zero U/O-flow indicates failure of MC matching for reco tracks, which results in PID=nan."));
  h_pid->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_deltalogl->GetListOfFunctions()->Add(new TNamed("Description",
                                                    TString::Format("ECL distribution of binary deltaLogL=logl(bkg)-logl(sig) - Signal PDG = %s, Bkg abs(PDG) = 11 | 321",
                                                        pdgIdStr.c_str()).Data()));
  h_deltalogl->GetListOfFunctions()->Add(new TNamed("Check",
                                                    "Basic metric for signal/bkg separation. The more negative, the better separation is achieved. Entries in U-flow indicate a non-normal PDF value (of sig OR bkg) for some p,clusterTheta range, which might be due to a non-optimal definition of the x-axis range of the PDF templates."));
  h_deltalogl->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_deltalogl->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_pid->Write();
  h_deltalogl->Write();

  delete h_pid;
  delete h_deltalogl;

}

void ECLChargedPIDDataAnalysisValidationModule::paintUnderOverflow(TH1F* h)
{

  auto nentries = h->GetEntries();
  auto nbins_vis = h->GetNbinsX();

  // Get the content and error of first/last visible bin.
  float bin_vis_first = h->GetBinContent(1);
  float bin_vis_last = h->GetBinContent(nbins_vis);
  float bin_vis_first_err = h->GetBinError(1);
  float bin_vis_last_err = h->GetBinError(nbins_vis);

  // Get the content and error of u/oflow bins.
  float bin_uflow = h->GetBinContent(0);
  float bin_oflow = h->GetBinContent(nbins_vis + 1);
  float bin_uflow_err = h->GetBinError(0);
  float bin_oflow_err = h->GetBinError(nbins_vis + 1);

  // Reset first/last visible bins to include u/oflow.
  h->SetBinContent(1, bin_vis_first + bin_uflow);
  h->SetBinError(1, sqrt(bin_vis_first_err * bin_vis_first_err + bin_uflow_err * bin_uflow_err));
  h->SetBinContent(nbins_vis, bin_vis_last + bin_oflow);
  h->SetBinError(nbins_vis, sqrt(bin_vis_last_err * bin_vis_last_err + bin_oflow_err * bin_oflow_err));

  // Reset total entries to the original value.
  h->SetEntries(nentries);

}


void ECLChargedPIDDataAnalysisValidationModule::dumpPIDEfficiency(TTree* tree, const std::string& pdgIdStr)
{

  // Histograms of p, clusterTheta, clusterPhi... for pass/all events.
  std::string h_p_N_name = "h_p_N_" + pdgIdStr;
  std::string h_p_D_name = "h_p_D_" + pdgIdStr;
  TH1F* h_p_N = new TH1F(h_p_N_name.c_str(), "h_p_N", 25, 0.0, 5.0);
  TH1F* h_p_D = new TH1F(h_p_D_name.c_str(), "h_p_D", 25, 0.0, 5.0);

  std::string h_th_N_name = "h_th_N_" + pdgIdStr;
  std::string h_th_D_name = "h_th_D_" + pdgIdStr;
  TH1F* h_th_N = new TH1F(h_th_N_name.c_str(), "h_th_N", m_th_binedges.size() - 1, m_th_binedges.data());
  TH1F* h_th_D = new TH1F(h_th_D_name.c_str(), "h_th_D", m_th_binedges.size() - 1, m_th_binedges.data());

  std::string h_phi_N_name = "h_phi_N_" + pdgIdStr;
  std::string h_phi_D_name = "h_phi_D_" + pdgIdStr;
  TH1F* h_phi_N = new TH1F(h_phi_N_name.c_str(), "h_phi_N", 30, -3.14159, 3.14159);
  TH1F* h_phi_D = new TH1F(h_phi_D_name.c_str(), "h_phi_D", 30, -3.14159, 3.14159);

  std::string pid_cut = "pid_glob > " + std::to_string(c_PID);

  tree->Project(h_p_N_name.c_str(), "p", pid_cut.c_str());
  tree->Project(h_p_D_name.c_str(), "p");

  tree->Project(h_th_N_name.c_str(), "clusterTheta", pid_cut.c_str());
  tree->Project(h_th_D_name.c_str(), "clusterTheta");

  tree->Project(h_phi_N_name.c_str(), "clusterPhi", pid_cut.c_str());
  tree->Project(h_phi_D_name.c_str(), "clusterPhi");

  // Compute the efficiency.

  std::string pid_eff_p_name = "pid_glob_eff_" + pdgIdStr + "__VS_p";
  std::string pid_eff_th_name = "pid_glob_eff_" + pdgIdStr + "__VS_th";
  std::string pid_eff_phi_name = "pid_glob_eff_" + pdgIdStr + "__VS_phi";

  if (TEfficiency::CheckConsistency(*h_p_N, *h_p_D)) {

    TEfficiency* t_pid_eff_p = new TEfficiency(*h_p_N, *h_p_D);
    t_pid_eff_p->SetName(pid_eff_p_name.c_str());
    t_pid_eff_p->SetTitle(pid_eff_p_name.c_str());

    t_pid_eff_p->SetConfidenceLevel(0.683);
    t_pid_eff_p->SetStatisticOption(TEfficiency::kBUniform);
    t_pid_eff_p->SetPosteriorMode();

    t_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Description",
                                                      TString::Format("Efficiency of ECL PID > 0.5 as a function of track momentum - Signal PDG = %s", pdgIdStr.c_str()).Data()));
    t_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Check",
                                                      "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
    t_pid_eff_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_pid_eff_p->Write();

    delete t_pid_eff_p;

  }
  if (TEfficiency::CheckConsistency(*h_th_N, *h_th_D)) {

    TEfficiency* t_pid_eff_th = new TEfficiency(*h_th_N, *h_th_D);
    t_pid_eff_th->SetName(pid_eff_th_name.c_str());
    t_pid_eff_th->SetTitle(pid_eff_th_name.c_str());

    t_pid_eff_th->SetConfidenceLevel(0.683);
    t_pid_eff_th->SetStatisticOption(TEfficiency::kBUniform);
    t_pid_eff_th->SetPosteriorMode();

    t_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Description",
                                                       TString::Format("Efficiency of ECL PID > 0.5 as a function of clusterTheta - Signal PDG = %s", pdgIdStr.c_str()).Data()));
    t_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Check",
                                                       "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
    t_pid_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_pid_eff_th->Write();

    delete t_pid_eff_th;

  }
  if (TEfficiency::CheckConsistency(*h_phi_N, *h_phi_D)) {

    TEfficiency* t_pid_eff_phi = new TEfficiency(*h_phi_N, *h_phi_D);
    t_pid_eff_phi->SetName(pid_eff_phi_name.c_str());
    t_pid_eff_phi->SetTitle(pid_eff_phi_name.c_str());

    t_pid_eff_phi->SetConfidenceLevel(0.683);
    t_pid_eff_phi->SetStatisticOption(TEfficiency::kBUniform);
    t_pid_eff_phi->SetPosteriorMode();

    t_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Description",
                                                        TString::Format("Efficiency of ECL PID > 0.5 as a function of clusterPhi - Signal PDG = %s", pdgIdStr.c_str()).Data()));
    t_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Check",
                                                        "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
    t_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_pid_eff_phi->Write();

    delete t_pid_eff_phi;
  }

  delete h_p_N;
  delete h_p_D;
  delete h_th_N;
  delete h_th_D;
  delete h_phi_N;
  delete h_phi_D;

}


void ECLChargedPIDDataAnalysisValidationModule::dumpTrkClusMatchingEfficiency(TTree* tree, const std::string& pdgIdStr)
{

  // Histograms of p, trkTheta, trkPhi... for pass/all events.
  std::string h_p_N_name = "h_p_N_" + pdgIdStr;
  std::string h_p_D_name = "h_p_D_" + pdgIdStr;
  TH1F* h_p_N = new TH1F(h_p_N_name.c_str(), "h_p_N", 25, 0.0, 5.0);
  TH1F* h_p_D = new TH1F(h_p_D_name.c_str(), "h_p_D", 25, 0.0, 5.0);

  std::string h_th_N_name = "h_th_N_" + pdgIdStr;
  std::string h_th_D_name = "h_th_D_" + pdgIdStr;
  TH1F* h_th_N = new TH1F(h_th_N_name.c_str(), "h_th_N", m_th_binedges.size() - 1, m_th_binedges.data());
  TH1F* h_th_D = new TH1F(h_th_D_name.c_str(), "h_th_D", m_th_binedges.size() - 1, m_th_binedges.data());

  std::string h_phi_N_name = "h_phi_N_" + pdgIdStr;
  std::string h_phi_D_name = "h_phi_D_" + pdgIdStr;
  TH1F* h_phi_N = new TH1F(h_phi_N_name.c_str(), "h_phi_N", 30, -3.14159, 3.14159);
  TH1F* h_phi_D = new TH1F(h_phi_D_name.c_str(), "h_phi_D", 30, -3.14159, 3.14159);

  std::string match_cut_N = "trackClusterMatch == 1";
  std::string match_cut_D = "trackClusterMatch >= 0";

  tree->Project(h_p_N_name.c_str(), "p", match_cut_N.c_str());
  tree->Project(h_p_D_name.c_str(), "p", match_cut_D.c_str());

  tree->Project(h_th_N_name.c_str(), "trkTheta", match_cut_N.c_str());
  tree->Project(h_th_D_name.c_str(), "trkTheta", match_cut_D.c_str());

  tree->Project(h_phi_N_name.c_str(), "trkPhi", match_cut_N.c_str());
  tree->Project(h_phi_D_name.c_str(), "trkPhi", match_cut_D.c_str());

  // Compute the efficiency.

  std::string match_eff_p_name = "trkclusmatch_eff_" + pdgIdStr + "__VS_p";
  std::string match_eff_th_name = "trkclusmatch_eff_" + pdgIdStr + "__VS_th";
  std::string match_eff_phi_name = "trkclusmatch_eff_" + pdgIdStr + "__VS_phi";

  if (TEfficiency::CheckConsistency(*h_p_N, *h_p_D)) {

    TEfficiency* t_match_eff_p = new TEfficiency(*h_p_N, *h_p_D);
    t_match_eff_p->SetName(match_eff_p_name.c_str());
    t_match_eff_p->SetTitle(match_eff_p_name.c_str());

    t_match_eff_p->SetConfidenceLevel(0.683);
    t_match_eff_p->SetStatisticOption(TEfficiency::kBUniform);
    t_match_eff_p->SetPosteriorMode();

    t_match_eff_p->GetListOfFunctions()->Add(new TNamed("Description",
                                                        TString::Format("Efficiency of track-eclcluster matching as a function of track momentum - Signal PDG = %s",
                                                            pdgIdStr.c_str()).Data()));
    t_match_eff_p->GetListOfFunctions()->Add(new TNamed("Check",
                                                        "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_match_eff_p->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
    t_match_eff_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_match_eff_p->Write();

    delete t_match_eff_p;

  }
  if (TEfficiency::CheckConsistency(*h_th_N, *h_th_D)) {

    TEfficiency* t_match_eff_th = new TEfficiency(*h_th_N, *h_th_D);
    t_match_eff_th->SetName(match_eff_th_name.c_str());
    t_match_eff_th->SetTitle(match_eff_th_name.c_str());

    t_match_eff_th->SetConfidenceLevel(0.683);
    t_match_eff_th->SetStatisticOption(TEfficiency::kBUniform);
    t_match_eff_th->SetPosteriorMode();

    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("Description",
                                                         TString::Format("Efficiency of track-eclcluster matching as a function of clusterTheta - Signal PDG = %s",
                                                             pdgIdStr.c_str()).Data()));
    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("Check",
                                                         "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_match_eff_th->Write();

    delete t_match_eff_th;

  }
  if (TEfficiency::CheckConsistency(*h_phi_N, *h_phi_D)) {

    TEfficiency* t_match_eff_phi = new TEfficiency(*h_phi_N, *h_phi_D);
    t_match_eff_phi->SetName(match_eff_phi_name.c_str());
    t_match_eff_phi->SetTitle(match_eff_phi_name.c_str());

    t_match_eff_phi->SetConfidenceLevel(0.683);
    t_match_eff_phi->SetStatisticOption(TEfficiency::kBUniform);
    t_match_eff_phi->SetPosteriorMode();

    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Description",
                                                          TString::Format("Efficiency of track-eclcluster matching as a function of clusterPhi - Signal PDG = %s", pdgIdStr.c_str()).Data()));
    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Check",
                                                          "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_match_eff_phi->Write();

    delete t_match_eff_phi;
  }

  delete h_p_N;
  delete h_p_D;
  delete h_th_N;
  delete h_th_D;
  delete h_phi_N;
  delete h_phi_D;

}
