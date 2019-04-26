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
  addParam("inputPdgId", m_inputPdgId,
           "The pdgId of the charged stable particle of interest.",
           int(0));
  addParam("outputFileName", m_outputFileName,
           "The name of the output file.",
           std::string("ECLChargedPid"));
}

ECLChargedPIDDataAnalysisValidationModule::~ECLChargedPIDDataAnalysisValidationModule()
{
}


void ECLChargedPIDDataAnalysisValidationModule::initialize()
{
  B2INFO("Initialising ROOT objects...");

  if (m_inputPdgId > 0) {
    m_inputPdgIdStr = std::to_string(m_inputPdgId);
  } else {
    m_inputPdgIdStr = "anti" + std::to_string(std::abs(m_inputPdgId));
  }

  std::string fname = m_outputFileName + "_" + m_inputPdgIdStr + ".root";

  m_outputFile = new TFile(fname.c_str(), "RECREATE");

  m_tree = new TTree("ECLChargedPid", "ECLChargedPid");
  m_tree->Branch("p", &m_p, "p/F");
  m_tree->Branch("clusterTheta", &m_clusterTheta, "clusterTheta/F");
  m_tree->Branch("clusterPhi", &m_clusterPhi, "clusterPhi/F");
  m_tree->Branch("trackClusterMatch", &m_trackClusterMatch, "trackClusterMatch/B");
  m_tree->Branch("pid", &m_pid, "pid/F");

}

void ECLChargedPIDDataAnalysisValidationModule::beginRun()
{
}

void ECLChargedPIDDataAnalysisValidationModule::event()
{

  // Initialise branches to unphysical values.
  m_p = -1.0;
  m_clusterTheta = -1.0;
  m_clusterPhi = -4.0;
  m_trackClusterMatch = -1;
  m_pid = -1.0;

  StoreArray<MCParticle> particles;

  for (const auto& particle : particles) {

    if (!particle.hasStatus(MCParticle::c_PrimaryParticle)) continue; // Only check primaries.
    if (particle.hasStatus(MCParticle::c_Initial)) continue; // Ignore initial particles.
    if (particle.hasStatus(MCParticle::c_IsVirtual)) continue; // Ignore virtual particles.

    // Skip all particles expect for the one of interest.
    if (particle.getPDG() != m_inputPdgId) continue;

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
      m_trackClusterMatch = 0;
      continue;
    }

    const auto eclCluster = eclClusters[icluster_match];

    const auto eclLikelihood = track->getRelated<ECLPidLikelihood>();

    double lh_sig = eclLikelihood->getLikelihood(Const::chargedStableSet.find(std::abs(m_inputPdgId)));

    double lh_all(0);
    for (const auto& chargedStable : Const::chargedStableSet) {
      lh_all += eclLikelihood->getLikelihood(chargedStable);
    }

    double pid = lh_sig / lh_all;

    // Fill the tree branches.
    m_p = p_max;
    m_clusterTheta = eclCluster->getTheta();
    m_clusterPhi = eclCluster->getPhi();
    m_trackClusterMatch = 1;
    m_pid = pid;

  }

  m_tree->Fill();

}

void ECLChargedPIDDataAnalysisValidationModule::endRun()
{
}

void ECLChargedPIDDataAnalysisValidationModule::terminate()
{

  computePIDEfficiency();
  computeMatchingEfficiency();

  m_outputFile->cd();
  m_tree->Write();
  m_outputFile->Close();
}


void ECLChargedPIDDataAnalysisValidationModule::computePIDEfficiency()
{

  m_outputFile->cd();

  TH1F* h_p_N = new TH1F("h_p_N", "h_p_N", m_p_binedges.size() - 1, m_p_binedges.data());
  TH1F* h_p_D = new TH1F("h_p_D", "h_p_D", m_p_binedges.size() - 1, m_p_binedges.data());

  TH1F* h_th_N = new TH1F("h_th_N", "h_th_N", m_th_binedges.size() - 1, m_th_binedges.data());
  TH1F* h_th_D = new TH1F("h_th_D", "h_th_D", m_th_binedges.size() - 1, m_th_binedges.data());

  TH1F* h_phi_N = new TH1F("h_phi_N", "h_phi_N", 60, -3.14159, 3.14159);
  TH1F* h_phi_D = new TH1F("h_phi_D", "h_phi_D", 60, -3.14159, 3.14159);

  // Just to get rid of warnings.
  h_p_N = h_p_N;
  h_p_D = h_p_D;
  h_th_N = h_th_N;
  h_th_D = h_th_D;
  h_phi_N = h_phi_N;
  h_phi_D = h_phi_D;

  std::string pid_cut = "pid > " + std::to_string(c_PID);

  m_tree->Project("h_p_N", "p", pid_cut.c_str());
  m_tree->Project("h_p_D", "p");

  m_tree->Project("h_th_N", "clusterTheta", pid_cut.c_str());
  m_tree->Project("h_th_D", "clusterTheta");

  m_tree->Project("h_phi_N", "clusterPhi", pid_cut.c_str());
  m_tree->Project("h_phi_D", "clusterPhi");

  // Compute the efficiency.

  std::string pid_eff_p_name = "pid_eff_" + m_inputPdgIdStr + "__VS_p";
  std::string pid_eff_th_name = "pid_eff_" + m_inputPdgIdStr + "__VS_th";
  std::string pid_eff_phi_name = "pid_eff_" + m_inputPdgIdStr + "__VS_phi";

  // Use TH1::Divide with binomial errors.
  TH1F* h_pid_eff_p = dynamic_cast<TH1F*>(h_p_N->Clone(pid_eff_p_name.c_str()));
  h_pid_eff_p->Divide(h_p_N, h_p_D, 1.0, 1.0, "B");

  TH1F* h_pid_eff_th = dynamic_cast<TH1F*>(h_th_N->Clone(pid_eff_th_name.c_str()));
  h_pid_eff_th->Divide(h_th_N, h_th_D, 1.0, 1.0, "B");

  TH1F* h_pid_eff_phi = dynamic_cast<TH1F*>(h_phi_N->Clone(pid_eff_phi_name.c_str()));
  h_pid_eff_phi->Divide(h_phi_N, h_phi_D, 1.0, 1.0, "B");

  // Add histogram info.

  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Description", "Efficiency of ECL PID > 0.5 as a function of track momentum."));
  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid_eff_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Description", "Efficiency of ECL PID > 0.5 as a function of clusterTheta."));
  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Description", "Efficiency of ECL PID > 0.5 as a function of clusterPhi."));
  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent and flat."));
  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_pid_eff_p->Write();
  h_pid_eff_th->Write();
  h_pid_eff_phi->Write();

  // No chi-2 test available when using TEfficiency. Just kept for reference.
  if (TEfficiency::CheckConsistency(*h_p_N, *h_p_D)) {

    TEfficiency* tpid_eff_p = new TEfficiency(*h_p_N, *h_p_D);
    pid_eff_p_name = "t_" + pid_eff_p_name;
    tpid_eff_p->SetName(pid_eff_p_name.c_str());

    tpid_eff_p->SetConfidenceLevel(0.683);
    tpid_eff_p->SetStatisticOption(TEfficiency::kBUniform);
    tpid_eff_p->SetPosteriorMode();

    tpid_eff_p->Write();

  }
  if (TEfficiency::CheckConsistency(*h_th_N, *h_th_D)) {

    TEfficiency* tpid_eff_th = new TEfficiency(*h_th_N, *h_th_D);
    pid_eff_th_name = "t_" + pid_eff_th_name;
    tpid_eff_th->SetName(pid_eff_th_name.c_str());

    tpid_eff_th->SetConfidenceLevel(0.683);
    tpid_eff_th->SetStatisticOption(TEfficiency::kBUniform);
    tpid_eff_th->SetPosteriorMode();

    m_outputFile->cd();
    tpid_eff_th->Write();

  }
  if (TEfficiency::CheckConsistency(*h_phi_N, *h_phi_D)) {

    TEfficiency* tpid_eff_phi = new TEfficiency(*h_phi_N, *h_phi_D);
    pid_eff_phi_name = "t_" + pid_eff_phi_name;
    tpid_eff_phi->SetName(pid_eff_phi_name.c_str());

    tpid_eff_phi->SetConfidenceLevel(0.683);
    tpid_eff_phi->SetStatisticOption(TEfficiency::kBUniform);
    tpid_eff_phi->SetPosteriorMode();

    m_outputFile->cd();
    tpid_eff_phi->Write();

  }

}

void ECLChargedPIDDataAnalysisValidationModule::computeMatchingEfficiency()
{
  m_outputFile->cd();

  TH1F* h_p_N = new TH1F("h_p_N", "h_p_N", 50, 0.0, 5.0);
  TH1F* h_p_D = new TH1F("h_p_D", "h_p_D", 50, 0.0, 5.0);

  TH1F* h_th_N = new TH1F("h_th_N", "h_th_N", m_th_binedges.size() - 1, m_th_binedges.data());
  TH1F* h_th_D = new TH1F("h_th_D", "h_th_D", m_th_binedges.size() - 1, m_th_binedges.data());

  TH1F* h_phi_N = new TH1F("h_phi_N", "h_phi_N", 60, -3.14159, 3.14159);
  TH1F* h_phi_D = new TH1F("h_phi_D", "h_phi_D", 60, -3.14159, 3.14159);

  // Just to get rid of warnings.
  h_p_N = h_p_N;
  h_p_D = h_p_D;
  h_th_N = h_th_N;
  h_th_D = h_th_D;
  h_phi_N = h_phi_N;
  h_phi_D = h_phi_D;

  std::string match_cut_N = "trackClusterMatch == 1";
  std::string match_cut_D = "trackClusterMatch >= 0";

  m_tree->Project("h_p_N", "p", match_cut_N.c_str());
  m_tree->Project("h_p_D", "p", match_cut_D.c_str());

  m_tree->Project("h_th_N", "clusterTheta", match_cut_N.c_str());
  m_tree->Project("h_th_D", "clusterTheta", match_cut_D.c_str());

  m_tree->Project("h_phi_N", "clusterPhi", match_cut_N.c_str());
  m_tree->Project("h_phi_D", "clusterPhi", match_cut_D.c_str());

  // Compute the efficiency.

  std::string match_eff_p_name = "match_eff_" + m_inputPdgIdStr + "__VS_p";
  std::string match_eff_th_name = "match_eff_" + m_inputPdgIdStr + "__VS_th";
  std::string match_eff_phi_name = "match_eff_" + m_inputPdgIdStr + "__VS_phi";

  // Use TH1::Divide with binomial errors.
  TH1F* h_match_eff_p = dynamic_cast<TH1F*>(h_p_N->Clone(match_eff_p_name.c_str()));
  h_match_eff_p->Divide(h_p_N, h_p_D, 1.0, 1.0, "B");

  TH1F* h_match_eff_th = dynamic_cast<TH1F*>(h_th_N->Clone(match_eff_th_name.c_str()));
  h_match_eff_th->Divide(h_th_N, h_th_D, 1.0, 1.0, "B");

  TH1F* h_match_eff_phi = dynamic_cast<TH1F*>(h_phi_N->Clone(match_eff_phi_name.c_str()));
  h_match_eff_phi->Divide(h_phi_N, h_phi_D, 1.0, 1.0, "B");

  // Add histogram info.

  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("Description",
                                                      "Efficiency of track-cluster matching as a function of track momentum."));
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_match_eff_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("Description",
                                                       "Efficiency of track-cluster matching as a function of clusterTheta."));
  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent."));
  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_match_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Description",
                                                        "Efficiency of track-cluster matching as a function of clusterPhi."));
  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Check", "Shape should be consistent and flat."));
  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_match_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_match_eff_p->Write();
  h_match_eff_th->Write();
  h_match_eff_phi->Write();

}
