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
    m_tree[chargedIdx]->Branch("pid", &m_pid[chargedIdx], "pid/F");

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
      // Add offset to idx.
      chargedIdx += Const::ChargedStable::c_SetSize;
    }

    // Initialise branches to unphysical values.
    m_p[chargedIdx] = -1.0;
    m_trkTheta[chargedIdx] = -1.0;
    m_trkPhi[chargedIdx] = -4.0;
    m_clusterTheta[chargedIdx] = -1.0;
    m_clusterPhi[chargedIdx] = -4.0;
    m_trackClusterMatch[chargedIdx] = -1;
    m_pid[chargedIdx] = -1.0;

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

      // Get the ECL cluster matching this track.
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

      double lh_sig = eclLikelihood->getLikelihood(Const::chargedStableSet.find(std::abs(chargedPdgId)));

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
      // Add offset to idx.
      chargedIdx += Const::ChargedStable::c_SetSize;
    }

    m_outputFile[chargedIdx]->cd();

    computePIDEfficiency(m_tree[chargedIdx], chargedPdgIdStr);
    computeMatchingEfficiency(m_tree[chargedIdx], chargedPdgIdStr);

    m_tree[chargedIdx]->Write();

    m_outputFile[chargedIdx]->Close();

  }
}


void ECLChargedPIDDataAnalysisValidationModule::computePIDEfficiency(TTree* tree, const std::string& pdgIdStr)
{

  // Histogram of PID distribution.
  std::string h_pid_name = "h_pid_" + pdgIdStr;
  TH1F* h_pid = new TH1F(h_pid_name.c_str(), h_pid_name.c_str(), 100, -1.2, 1.2);

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

  // Just to get rid of warnings.
  h_pid = h_pid;
  h_p_N = h_p_N;
  h_p_D = h_p_D;
  h_th_N = h_th_N;
  h_th_D = h_th_D;
  h_phi_N = h_phi_N;
  h_phi_D = h_phi_D;

  tree->Project(h_pid_name.c_str(), "pid");

  std::string pid_cut = "pid > " + std::to_string(c_PID);

  tree->Project(h_p_N_name.c_str(), "p", pid_cut.c_str());
  tree->Project(h_p_D_name.c_str(), "p");

  tree->Project(h_th_N_name.c_str(), "clusterTheta", pid_cut.c_str());
  tree->Project(h_th_D_name.c_str(), "clusterTheta");

  tree->Project(h_phi_N_name.c_str(), "clusterPhi", pid_cut.c_str());
  tree->Project(h_phi_D_name.c_str(), "clusterPhi");

  // Compute the efficiency.
  // Use TH1::Divide with binomial errors.

  std::string pid_eff_p_name = "pid_eff_" + pdgIdStr + "__VS_p";
  std::string pid_eff_th_name = "pid_eff_" + pdgIdStr + "__VS_th";
  std::string pid_eff_phi_name = "pid_eff_" + pdgIdStr + "__VS_phi";

  TH1F* h_pid_eff_p = dynamic_cast<TH1F*>(h_p_N->Clone(pid_eff_p_name.c_str()));
  h_pid_eff_p->SetTitle(pid_eff_p_name.c_str());
  h_pid_eff_p->Divide(h_p_N, h_p_D, 1.0, 1.0, "B");
  h_pid_eff_p->GetYaxis()->SetRangeUser(0.0, 1.1);

  TH1F* h_pid_eff_th = dynamic_cast<TH1F*>(h_th_N->Clone(pid_eff_th_name.c_str()));
  h_pid_eff_th->SetTitle(pid_eff_th_name.c_str());
  h_pid_eff_th->Divide(h_th_N, h_th_D, 1.0, 1.0, "B");
  h_pid_eff_th->GetYaxis()->SetRangeUser(0.0, 1.1);

  TH1F* h_pid_eff_phi = dynamic_cast<TH1F*>(h_phi_N->Clone(pid_eff_phi_name.c_str()));
  h_pid_eff_phi->SetTitle(pid_eff_phi_name.c_str());
  h_pid_eff_phi->Divide(h_phi_N, h_phi_D, 1.0, 1.0, "B");
  h_pid_eff_phi->GetYaxis()->SetRangeUser(0.0, 1.1);

  // Add histogram info.
  h_pid->GetListOfFunctions()->Add(new TNamed("Description", "ECL global PID distribution"));
  h_pid->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_pid->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Description", "Efficiency of ECL PID > 0.5 as a function of track momentum."));
  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Description", "Efficiency of ECL PID > 0.5 as a function of clusterTheta."));
  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Description", "Efficiency of ECL PID > 0.5 as a function of clusterPhi."));
  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

  h_pid->Write();
  h_pid_eff_p->Write();
  h_pid_eff_th->Write();
  h_pid_eff_phi->Write();

  // No chi-2 test available when using TEfficiency. Just kept for reference.
  if (TEfficiency::CheckConsistency(*h_p_N, *h_p_D)) {

    TEfficiency* tpid_eff_p = new TEfficiency(*h_p_N, *h_p_D);
    pid_eff_p_name = "t_" + pid_eff_p_name + "_" + pdgIdStr;
    tpid_eff_p->SetName(pid_eff_p_name.c_str());

    tpid_eff_p->SetConfidenceLevel(0.683);
    tpid_eff_p->SetStatisticOption(TEfficiency::kBUniform);
    tpid_eff_p->SetPosteriorMode();

    tpid_eff_p->Write();

    delete tpid_eff_p;

  }
  if (TEfficiency::CheckConsistency(*h_th_N, *h_th_D)) {

    TEfficiency* tpid_eff_th = new TEfficiency(*h_th_N, *h_th_D);
    pid_eff_th_name = "t_" + pid_eff_th_name + "_" + pdgIdStr;
    tpid_eff_th->SetName(pid_eff_th_name.c_str());

    tpid_eff_th->SetConfidenceLevel(0.683);
    tpid_eff_th->SetStatisticOption(TEfficiency::kBUniform);
    tpid_eff_th->SetPosteriorMode();

    tpid_eff_th->Write();

    delete tpid_eff_th;

  }
  if (TEfficiency::CheckConsistency(*h_phi_N, *h_phi_D)) {

    TEfficiency* tpid_eff_phi = new TEfficiency(*h_phi_N, *h_phi_D);
    pid_eff_phi_name = "t_" + pid_eff_phi_name + "_" + pdgIdStr;
    tpid_eff_phi->SetName(pid_eff_phi_name.c_str());

    tpid_eff_phi->SetConfidenceLevel(0.683);
    tpid_eff_phi->SetStatisticOption(TEfficiency::kBUniform);
    tpid_eff_phi->SetPosteriorMode();

    tpid_eff_phi->Write();

    delete tpid_eff_phi;
  }

  delete h_pid;
  delete h_p_N;
  delete h_p_D;
  delete h_th_N;
  delete h_th_D;
  delete h_phi_N;
  delete h_phi_D;

}

void ECLChargedPIDDataAnalysisValidationModule::computeMatchingEfficiency(TTree* tree, const std::string& pdgIdStr)
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

  // Just to get rid of warnings.
  h_p_N = h_p_N;
  h_p_D = h_p_D;
  h_th_N = h_th_N;
  h_th_D = h_th_D;
  h_phi_N = h_phi_N;
  h_phi_D = h_phi_D;

  std::string match_cut_N = "trackClusterMatch == 1";
  std::string match_cut_D = "trackClusterMatch >= 0";

  tree->Project(h_p_N_name.c_str(), "p", match_cut_N.c_str());
  tree->Project(h_p_D_name.c_str(), "p", match_cut_D.c_str());

  tree->Project(h_th_N_name.c_str(), "trkTheta", match_cut_N.c_str());
  tree->Project(h_th_D_name.c_str(), "trkTheta", match_cut_D.c_str());

  tree->Project(h_phi_N_name.c_str(), "trkPhi", match_cut_N.c_str());
  tree->Project(h_phi_D_name.c_str(), "trkPhi", match_cut_D.c_str());

  // Compute the efficiency.
  // Use TH1::Divide with binomial errors.

  std::string match_eff_p_name = "trkclusmatch_eff_" + pdgIdStr + "__VS_p";
  std::string match_eff_th_name = "trkclusmatch_eff_" + pdgIdStr + "__VS_th";
  std::string match_eff_phi_name = "trkclusmatch_eff_" + pdgIdStr + "__VS_phi";

  TH1F* h_match_eff_p = dynamic_cast<TH1F*>(h_p_N->Clone(match_eff_p_name.c_str()));
  h_match_eff_p->SetTitle(match_eff_p_name.c_str());
  h_match_eff_p->Divide(h_p_N, h_p_D, 1.0, 1.0, "B");
  h_match_eff_p->GetYaxis()->SetRangeUser(0.0, 1.1);

  TH1F* h_match_eff_th = dynamic_cast<TH1F*>(h_th_N->Clone(match_eff_th_name.c_str()));
  h_match_eff_th->SetTitle(match_eff_th_name.c_str());
  h_match_eff_th->Divide(h_th_N, h_th_D, 1.0, 1.0, "B");
  h_match_eff_th->GetYaxis()->SetRangeUser(0.0, 1.1);

  TH1F* h_match_eff_phi = dynamic_cast<TH1F*>(h_phi_N->Clone(match_eff_phi_name.c_str()));
  h_match_eff_phi->SetTitle(match_eff_phi_name.c_str());
  h_match_eff_phi->Divide(h_phi_N, h_phi_D, 1.0, 1.0, "B");
  h_match_eff_phi->GetYaxis()->SetRangeUser(0.0, 1.1);

  // Add histogram info.
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("Description",
                                                      "Efficiency of track-cluster matching as a function of track momentum."));
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("Description",
                                                       "Efficiency of track-cluster matching as a function of clusterTheta."));
  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Description",
                                                        "Efficiency of track-cluster matching as a function of clusterPhi."));
  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

  h_match_eff_p->Write();
  h_match_eff_th->Write();
  h_match_eff_phi->Write();

  delete h_p_N;
  delete h_p_D;
  delete h_th_N;
  delete h_th_D;
  delete h_phi_N;
  delete h_phi_D;

}
