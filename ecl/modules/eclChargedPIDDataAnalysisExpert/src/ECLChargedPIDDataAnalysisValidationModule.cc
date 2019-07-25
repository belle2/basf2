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
  setDescription("This module dumps a set of histograms with ECL charged PID-related info used for validation, starting from an input file w/ particle-gun-generated charged stable particles (and antiparticles).");

  // Default charged stable pdgIds (particles & antiparticles)
  std::vector<int> defaultChargedPdgIds;
  for (const auto& hypo : Const::chargedStableSet) {
    defaultChargedPdgIds.push_back(hypo.getPDGCode());
    defaultChargedPdgIds.push_back(-hypo.getPDGCode());
  }

  addParam("inputPdgIdList", m_inputPdgIdList,
           "The list of (signed) pdgIds of the charged stable particles for which validation plots should be produced. Default is ALL charged stable particles.",
           defaultChargedPdgIds);
  addParam("outputFileName", m_outputFileName,
           "The base name of the output file. The pdgId of the charged particle is appended to the name.",
           std::string("ECLChargedPid"));
  addParam("saveValidationTree", m_saveValidationTree,
           "If this flag is set to True, save also the validation TTree. Default is False.",
           bool(false));
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
    m_tree[chargedIdx]->Branch("pt", &m_pt[chargedIdx], "pt/F");
    m_tree[chargedIdx]->Branch("trkTheta", &m_trkTheta[chargedIdx], "trkTheta/F");
    m_tree[chargedIdx]->Branch("trkPhi", &m_trkPhi[chargedIdx], "trkPhi/F");
    m_tree[chargedIdx]->Branch("clusterTheta", &m_clusterTheta[chargedIdx], "clusterTheta/F");
    m_tree[chargedIdx]->Branch("clusterPhi", &m_clusterPhi[chargedIdx], "clusterPhi/F");
    m_tree[chargedIdx]->Branch("trackClusterMatch", &m_trackClusterMatch[chargedIdx], "trackClusterMatch/B");
    m_tree[chargedIdx]->Branch("logl_sig", &m_logl_sig[chargedIdx], "logl_sig/F");
    m_tree[chargedIdx]->Branch("logl_bkg", &m_logl_bkg[chargedIdx], "logl_bkg/F");
    m_tree[chargedIdx]->Branch("deltalogl_sig_bkg", &m_deltalogl_sig_bkg[chargedIdx], "deltalogl_sig_bkg/F");
    m_tree[chargedIdx]->Branch("pids_glob", &m_pids_glob[chargedIdx]);

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
    m_pt[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_trkTheta[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_trkPhi[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_clusterTheta[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_clusterPhi[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_trackClusterMatch[chargedIdx] = std::numeric_limits<char>::quiet_NaN();
    m_logl_sig[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_logl_bkg[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    m_deltalogl_sig_bkg[chargedIdx] = std::numeric_limits<float>::quiet_NaN();
    for (const auto& chargedStable : Const::chargedStableSet) {
      m_pids_glob[chargedIdx][chargedStable.getIndex()] = std::numeric_limits<float>::quiet_NaN();
    }

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
      m_pt[chargedIdx] = fitRes->get4Momentum().Pt();
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

      // The "signal" likelihood corresponds to the current chargedPdgId.
      const auto chargedStableSig = Const::chargedStableSet.find(std::abs(chargedPdgId));
      // For deltaLogL, we do a binary comparison sig/bkg.
      // If sig=pion, use bkg=kaon. Otherwise, bkg=pion.
      const auto chargedStableBkg = (chargedStableSig == Const::pion) ? Const::kaon : Const::pion;

      double lh_sig = eclLikelihood->getLikelihood(chargedStableSig);
      double lh_bkg = eclLikelihood->getLikelihood(chargedStableBkg);

      m_logl_sig[chargedIdx] = log(lh_sig);
      m_logl_bkg[chargedIdx] = log(lh_bkg);
      m_deltalogl_sig_bkg[chargedIdx] = log(lh_bkg) - log(lh_sig);

      // For the current charged particle candidate, store the global likelihood ratio for all hypotheses.
      double lh_all(0);
      for (const auto& chargedStable : Const::chargedStableSet) {
        lh_all += eclLikelihood->getLikelihood(chargedStable);
      }
      for (const auto& chargedStable : Const::chargedStableSet) {
        m_pids_glob[chargedIdx][chargedStable.getIndex()] = eclLikelihood->getLikelihood(chargedStable) / lh_all;
      }

    }

    m_tree[chargedIdx]->Fill();

  }
}

void ECLChargedPIDDataAnalysisValidationModule::endRun()
{
}

void ECLChargedPIDDataAnalysisValidationModule::terminate()
{

  for (const auto& chargedPdgId : m_inputPdgIdSet) {

    // Extract the sign of the charge.
    const auto chargeSign = static_cast<int>(chargedPdgId / std::abs(chargedPdgId));

    // Define the charged stable particle ("sample") corresponding to the current pdgId.
    const auto chargedStableSample = Const::chargedStableSet.find(std::abs(chargedPdgId));

    // What we call "signal" is equal to the sample under consideration.
    const auto chargedStableSig = chargedStableSample;

    // What we call "background" depends on the current "signal" particle hypothesis.
    const auto chargedStableBkg = (chargedStableSig == Const::pion) ? Const::kaon : Const::pion;

    // Get the idx of this sample's pdgId to retrieve the correct TTree and output TFile.
    // Remember to add offset for antiparticles.
    auto chargedSampleIdx = (chargeSign > 0) ? chargedStableSig.getIndex() : chargedStableSig.getIndex() +
                            Const::ChargedStable::c_SetSize;

    m_outputFile[chargedSampleIdx]->cd();

    // Add summary description of validation file content.
    TNamed("Description", TString::Format("ECL Charged PID control plots for charged stable particles/antiparticles - Sample PDG = %i",
                                          chargedPdgId).Data()).Write();

    // Dump plots of PID variables.
    dumpPIDVars(m_tree[chargedSampleIdx], chargedStableSig, chargeSign, chargedStableBkg);
    // Dump plots of PID "signal" efficiency for this "sample".
    dumpPIDEfficiencyFakeRate(m_tree[chargedSampleIdx], chargedStableSample, chargeSign, chargedStableSig);
    // For pions, dump also the pi->lep fake rate.
    if (chargedStableSample == Const::pion) {
      dumpPIDEfficiencyFakeRate(m_tree[chargedSampleIdx], chargedStableSample, chargeSign, Const::electron);
      dumpPIDEfficiencyFakeRate(m_tree[chargedSampleIdx], chargedStableSample, chargeSign, Const::muon);
    }
    // Dump plots of matching efficiency for this "sample".
    dumpTrkClusMatchingEfficiency(m_tree[chargedSampleIdx], chargedStableSample, chargeSign);

    // Write the TTree to file if requested.
    if (m_saveValidationTree) {
      m_tree[chargedSampleIdx]->Write();
    }

    m_outputFile[chargedSampleIdx]->Close();

  }
}

void ECLChargedPIDDataAnalysisValidationModule::dumpPIDVars(TTree* sampleTree, const Const::ChargedStable& sigHypo,
                                                            const int sigCharge, const Const::ChargedStable& bkgHypo)
{

  // Get the idx and pdgId of the input sample particle.
  // This corresponds by construction to the "signal" hypothesis for the likelihood and DeltaLogL.
  const auto sigHypoIdx = sigHypo.getIndex();
  const auto sigHypoPdgId = sigHypo.getPDGCode();

  // Get the pdgId of the "background" hypothesis to test for DeltaLogL.
  const auto bkgHypoPdgId = bkgHypo.getPDGCode();

  // Access the "signal" hypothesis's PID component in the sample's
  // TTree vector branch of global PID values via the idx.
  TString pidSigBranch = TString::Format("pids_glob[%i]", sigHypoIdx);

  // Histogram of global PID distribution for the sample particle's signal hypo.
  TString h_pid_name = TString::Format("h_pid_sig_%i", sigHypoPdgId);
  TH1F* h_pid = new TH1F(h_pid_name.Data(), h_pid_name.Data(), 50, -0.5, 1.2);
  h_pid->GetXaxis()->SetTitle(TString::Format("Likelihood ratio (%i/ALL) (ECL)", sigHypoPdgId).Data());

  // Histogram of deltalogl.
  TString h_deltalogl_name = TString::Format("h_deltalogl_bkg_%i_sig_%i", bkgHypoPdgId, sigHypoPdgId);
  double deltalogl_min = -20.0;
  double deltalogl_max = 20.0;
  TH1F* h_deltalogl = new TH1F(h_deltalogl_name.Data(), h_deltalogl_name.Data(), 40, deltalogl_min, deltalogl_max);
  h_deltalogl->GetXaxis()->SetTitle(TString::Format("#Deltaln(L) (%i/%i) (ECL)", bkgHypoPdgId, sigHypoPdgId).Data());

  // Dump histos from TTree.
  sampleTree->Project(h_pid_name.Data(), pidSigBranch.Data());
  sampleTree->Project(h_deltalogl_name.Data(), "deltalogl_sig_bkg");

  // Make sure the plots show the u/oflow.
  paintUnderOverflow(h_pid);
  paintUnderOverflow(h_deltalogl);

  h_pid->SetOption("HIST");
  h_deltalogl->SetOption("HIST");


  // Add histogram info.
  h_pid->GetListOfFunctions()->Add(new TNamed("Description", TString::Format("Sample PDG = %i - ECL global PID(%i) distribution.",
                                              sigHypoPdgId * sigCharge,
                                              sigHypoPdgId).Data()));
  h_pid->GetListOfFunctions()->Add(new TNamed("Check",
                                              "The more peaked at 1, the better. Non-zero U/O-flow indicates either failure of MC matching for reco tracks (unlikely) or of track-ECL-cluster matching (more likely). Both cases result in PID=nan."));
  h_pid->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_pid->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_deltalogl->GetListOfFunctions()->Add(new TNamed("Description",
                                                    TString::Format("Sample PDG = %i - ECL distribution of binary deltaLogL=logl(%i)-logl(%i)",
                                                        sigHypoPdgId * sigCharge,
                                                        bkgHypoPdgId,
                                                        sigHypoPdgId).Data()));
  h_deltalogl->GetListOfFunctions()->Add(new TNamed("Check",
                                                    "Basic metric for signal/bkg separation. The more negative, the better separation is achieved. Entries in U-flow indicate a non-normal PDF value (of sig OR bkg) for some p,clusterTheta range, which might be due to a non-optimal definition of the x-axis range of the PDF templates."));
  h_deltalogl->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
  h_deltalogl->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01"));

  h_pid->Write();
  h_deltalogl->Write();

  delete h_pid;
  delete h_deltalogl;

}


void ECLChargedPIDDataAnalysisValidationModule::dumpPIDEfficiencyFakeRate(TTree* sampleTree, const Const::ChargedStable& sampleHypo,
    const int sampleCharge, const Const::ChargedStable& sigHypo)
{

  // The ratio type: EFFICIENCY || FAKE RATE
  const std::string ratioType = (sampleHypo == sigHypo) ? "Efficiency" : "FakeRate";

  // Get the *signed* pdgId of the input sample particle.
  const int sampleHypoPdgId = sampleHypo.getPDGCode() * sampleCharge;

  // Get the idx and pdgId of the "signal" hypothesis to test.
  const auto sigHypoIdx = sigHypo.getIndex();
  const auto sigHypoPdgId = sigHypo.getPDGCode();

  // Access the "signal" hypothesis's PID component in the sample's
  // TTree vector branch of global PID values via the idx.
  TString pidSigCut = TString::Format("pids_glob[%i] > %f", sigHypoIdx, c_PID);

  // Histograms of p, clusterTheta, clusterPhi... for "pass" (N, numerator) and "all" (D, denominator) events.
  TString h_p_N_name = TString::Format("h_p_N_%i", sigHypoPdgId);
  TString h_p_D_name = TString::Format("h_p_D_%i", sigHypoPdgId);
  TH1F* h_p_N = new TH1F(h_p_N_name.Data(), "h_p_N", 25, 0.0, 5.0);
  TH1F* h_p_D = new TH1F(h_p_D_name.Data(), "h_p_D", 25, 0.0, 5.0);

  TString h_th_N_name = TString::Format("h_th_N_%i", sigHypoPdgId);
  TString h_th_D_name = TString::Format("h_th_D_%i", sigHypoPdgId);
  TH1F* h_th_N = new TH1F(h_th_N_name.Data(), "h_th_N", m_th_binedges.size() - 1, m_th_binedges.data());
  TH1F* h_th_D = new TH1F(h_th_D_name.Data(), "h_th_D", m_th_binedges.size() - 1, m_th_binedges.data());

  TString h_phi_N_name = TString::Format("h_phi_N_%i", sigHypoPdgId);
  TString h_phi_D_name = TString::Format("h_phi_D_%i", sigHypoPdgId);
  TH1F* h_phi_N = new TH1F(h_phi_N_name.Data(), "h_phi_N", 30, -3.14159, 3.14159);
  TH1F* h_phi_D = new TH1F(h_phi_D_name.Data(), "h_phi_D", 30, -3.14159, 3.14159);

  // Fill the histograms from the sample's TTree.

  sampleTree->Project(h_p_N_name.Data(), "p", pidSigCut.Data());
  sampleTree->Project(h_p_D_name.Data(), "p");

  sampleTree->Project(h_th_N_name.Data(), "clusterTheta", pidSigCut.Data());
  sampleTree->Project(h_th_D_name.Data(), "clusterTheta");

  sampleTree->Project(h_phi_N_name.Data(), "clusterPhi", pidSigCut.Data());
  sampleTree->Project(h_phi_D_name.Data(), "clusterPhi");

  // Compute the efficiency/fake rate.

  TString pid_glob_ratio_p_name = TString::Format("pid_glob_%i_%s__VS_p", sigHypoPdgId, ratioType.c_str());
  TString pid_glob_ratio_th_name = TString::Format("pid_glob_%i_%s__VS_th", sigHypoPdgId, ratioType.c_str());
  TString pid_glob_ratio_phi_name = TString::Format("pid_glob_%i_%s__VS_phi", sigHypoPdgId, ratioType.c_str());

  if (TEfficiency::CheckConsistency(*h_p_N, *h_p_D)) {

    TEfficiency* t_pid_glob_ratio_p = new TEfficiency(*h_p_N, *h_p_D);
    t_pid_glob_ratio_p->SetName(pid_glob_ratio_p_name.Data());
    t_pid_glob_ratio_p->SetTitle(TString::Format("%s;p [GeV/c];#varepsilon/f", pid_glob_ratio_p_name.Data()).Data());

    t_pid_glob_ratio_p->SetConfidenceLevel(0.683);
    t_pid_glob_ratio_p->SetStatisticOption(TEfficiency::kBUniform);
    t_pid_glob_ratio_p->SetPosteriorMode();

    t_pid_glob_ratio_p->GetListOfFunctions()->Add(new TNamed("Description",
                                                             TString::Format("Sample PDG = %i - %s of ECL global PID(%i) > %.2f as a function of track momentum.",
                                                                 sampleHypoPdgId,
                                                                 ratioType.c_str(),
                                                                 sigHypoPdgId,
                                                                 c_PID
                                                                            ).Data()));
    t_pid_glob_ratio_p->GetListOfFunctions()->Add(new TNamed("Check",
                                                             "Shape should be consistent. Obviously, check for decreasing efficiency / increasing fake rate."));
    t_pid_glob_ratio_p->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
    t_pid_glob_ratio_p->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_pid_glob_ratio_p->Write();

    delete t_pid_glob_ratio_p;

  }
  if (TEfficiency::CheckConsistency(*h_th_N, *h_th_D)) {

    TEfficiency* t_pid_glob_ratio_th = new TEfficiency(*h_th_N, *h_th_D);
    t_pid_glob_ratio_th->SetName(pid_glob_ratio_th_name.Data());
    t_pid_glob_ratio_th->SetTitle(TString::Format("%s;#theta_{cluster} [rad];#varepsilon/f", pid_glob_ratio_th_name.Data()).Data());

    t_pid_glob_ratio_th->SetConfidenceLevel(0.683);
    t_pid_glob_ratio_th->SetStatisticOption(TEfficiency::kBUniform);
    t_pid_glob_ratio_th->SetPosteriorMode();

    t_pid_glob_ratio_th->GetListOfFunctions()->Add(new TNamed("Description",
                                                              TString::Format("Sample PDG = %i - %s of ECL global PID(%i) > %.2f as a function of ECL cluster theta.",
                                                                  sampleHypoPdgId,
                                                                  ratioType.c_str(),
                                                                  sigHypoPdgId,
                                                                  c_PID
                                                                             ).Data()));
    t_pid_glob_ratio_th->GetListOfFunctions()->Add(new TNamed("Check",
                                                              "Shape should be consistent. Obviously, check for decreasing efficiency / increasing fake rate."));
    t_pid_glob_ratio_th->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
    t_pid_glob_ratio_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_pid_glob_ratio_th->Write();

    delete t_pid_glob_ratio_th;

  }
  if (TEfficiency::CheckConsistency(*h_phi_N, *h_phi_D)) {

    TEfficiency* t_pid_glob_ratio_phi = new TEfficiency(*h_phi_N, *h_phi_D);
    t_pid_glob_ratio_phi->SetName(pid_glob_ratio_phi_name.Data());
    t_pid_glob_ratio_phi->SetTitle(TString::Format("%s;#phi_{cluster} [rad];#varepsilon/f", pid_glob_ratio_phi_name.Data()).Data());

    t_pid_glob_ratio_phi->SetConfidenceLevel(0.683);
    t_pid_glob_ratio_phi->SetStatisticOption(TEfficiency::kBUniform);
    t_pid_glob_ratio_phi->SetPosteriorMode();

    t_pid_glob_ratio_phi->GetListOfFunctions()->Add(new TNamed("Description",
                                                               TString::Format("Sample PDG = %i - %s of ECL global PID(%i) > %.2f as a function of ECL cluster phi.",
                                                                   sampleHypoPdgId,
                                                                   ratioType.c_str(),
                                                                   sigHypoPdgId,
                                                                   c_PID
                                                                              ).Data()));
    t_pid_glob_ratio_phi->GetListOfFunctions()->Add(new TNamed("Check",
                                                               "Shape should be consistent. Obviously, check for decreasing efficiency / increasing fake rate."));
    t_pid_glob_ratio_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Marco Milesi. marco.milesi@desy.de"));
    t_pid_glob_ratio_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_pid_glob_ratio_phi->Write();

    delete t_pid_glob_ratio_phi;
  }

  delete h_p_N;
  delete h_p_D;
  delete h_th_N;
  delete h_th_D;
  delete h_phi_N;
  delete h_phi_D;

}


void ECLChargedPIDDataAnalysisValidationModule::dumpTrkClusMatchingEfficiency(TTree* tree, const Const::ChargedStable& sampleHypo,
    const int sampleCharge)
{

  // Get the (unsigned) pdgId of the input sample particle.
  const auto sampleHypoPdgId = sampleHypo.getPDGCode();

  // Histograms of pt, clusterTheta, clusterPhi... for "pass" (N, numerator) and "all" (D, denominator) events.
  TString h_pt_N_name = TString::Format("h_pt_N_%i", sampleHypoPdgId);
  TString h_pt_D_name = TString::Format("h_pt_D_%i", sampleHypoPdgId);
  TH1F* h_pt_N = new TH1F(h_pt_N_name.Data(), "h_pt_N", 25, 0.0, 5.0);
  TH1F* h_pt_D = new TH1F(h_pt_D_name.Data(), "h_pt_D", 25, 0.0, 5.0);

  TString h_th_N_name = TString::Format("h_th_N_%i", sampleHypoPdgId);
  TString h_th_D_name = TString::Format("h_th_D_%i", sampleHypoPdgId);
  TH1F* h_th_N = new TH1F(h_th_N_name.Data(), "h_th_N", m_th_binedges.size() - 1, m_th_binedges.data());
  TH1F* h_th_D = new TH1F(h_th_D_name.Data(), "h_th_D", m_th_binedges.size() - 1, m_th_binedges.data());

  TString h_phi_N_name = TString::Format("h_phi_N_%i", sampleHypoPdgId);
  TString h_phi_D_name = TString::Format("h_phi_D_%i", sampleHypoPdgId);
  TH1F* h_phi_N = new TH1F(h_phi_N_name.Data(), "h_phi_N", 30, -3.14159, 3.14159);
  TH1F* h_phi_D = new TH1F(h_phi_D_name.Data(), "h_phi_D", 30, -3.14159, 3.14159);

  TString match_cut_N("trackClusterMatch == 1");
  TString match_cut_D("trackClusterMatch >= 0");

  // Fill the histograms from the sample's TTree.

  tree->Project(h_pt_N_name.Data(), "pt", match_cut_N.Data());
  tree->Project(h_pt_D_name.Data(), "pt", match_cut_D.Data());

  tree->Project(h_th_N_name.Data(), "trkTheta", match_cut_N.Data());
  tree->Project(h_th_D_name.Data(), "trkTheta", match_cut_D.Data());

  tree->Project(h_phi_N_name.Data(), "trkPhi", match_cut_N.Data());
  tree->Project(h_phi_D_name.Data(), "trkPhi", match_cut_D.Data());

  // Compute the efficiency.

  TString match_eff_pt_name = TString::Format("trkclusmatch_%i_Efficiency__VS_pt", sampleHypoPdgId);
  TString match_eff_th_name = TString::Format("trkclusmatch_%i_Efficiency__VS_th", sampleHypoPdgId);
  TString match_eff_phi_name = TString::Format("trkclusmatch_%i_Efficiency__VS_phi", sampleHypoPdgId);

  if (TEfficiency::CheckConsistency(*h_pt_N, *h_pt_D)) {

    TEfficiency* t_match_eff_pt = new TEfficiency(*h_pt_N, *h_pt_D);
    t_match_eff_pt->SetName(match_eff_pt_name.Data());
    t_match_eff_pt->SetTitle(TString::Format("%s;p_{T} [GeV/c];#varepsilon", match_eff_pt_name.Data()).Data());
    t_match_eff_pt->SetTitle(match_eff_pt_name.Data());

    t_match_eff_pt->SetConfidenceLevel(0.683);
    t_match_eff_pt->SetStatisticOption(TEfficiency::kBUniform);
    t_match_eff_pt->SetPosteriorMode();

    t_match_eff_pt->GetListOfFunctions()->Add(new TNamed("Description",
                                                         TString::Format("Sample PDG = %i - Efficiency of track-ECL-cluster matching as a function of track transverse momentum.",
                                                             sampleHypoPdgId * sampleCharge).Data()));
    t_match_eff_pt->GetListOfFunctions()->Add(new TNamed("Check",
                                                         "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_match_eff_pt->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
    t_match_eff_pt->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_match_eff_pt->Write();

    delete t_match_eff_pt;

  }
  if (TEfficiency::CheckConsistency(*h_th_N, *h_th_D)) {

    TEfficiency* t_match_eff_th = new TEfficiency(*h_th_N, *h_th_D);
    t_match_eff_th->SetName(match_eff_th_name.Data());
    t_match_eff_th->SetTitle(TString::Format("%s;#theta_{cluster} [rad];#varepsilon", match_eff_th_name.Data()).Data());
    t_match_eff_th->SetTitle(match_eff_th_name.Data());

    t_match_eff_th->SetConfidenceLevel(0.683);
    t_match_eff_th->SetStatisticOption(TEfficiency::kBUniform);
    t_match_eff_th->SetPosteriorMode();

    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("Description",
                                                         TString::Format("Sample PDG = %i - Efficiency of track-ECL-cluster matching as a function of track theta.",
                                                             sampleHypoPdgId * sampleCharge).Data()));
    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("Check",
                                                         "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
    t_match_eff_th->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_match_eff_th->Write();

    delete t_match_eff_th;

  }
  if (TEfficiency::CheckConsistency(*h_phi_N, *h_phi_D)) {

    TEfficiency* t_match_eff_phi = new TEfficiency(*h_phi_N, *h_phi_D);
    t_match_eff_phi->SetName(match_eff_phi_name.Data());
    t_match_eff_phi->SetTitle(TString::Format("%s;#phi_{cluster} [rad];#varepsilon", match_eff_phi_name.Data()).Data());

    t_match_eff_phi->SetConfidenceLevel(0.683);
    t_match_eff_phi->SetStatisticOption(TEfficiency::kBUniform);
    t_match_eff_phi->SetPosteriorMode();

    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Description",
                                                          TString::Format("Sample PDG = %i - Efficiency of track-ECL-cluster matching as a function of track phi.",
                                                              sampleHypoPdgId * sampleCharge).Data()));
    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Check",
                                                          "Shape should be consistent. Obviously, check for decreasing efficiency."));
    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("Contact", "Frank Meier. frank.meier@desy.de"));
    t_match_eff_phi->GetListOfFunctions()->Add(new TNamed("MetaOptions", "pvalue-warn=0.5,pvalue-error=0.01,nostats"));

    t_match_eff_phi->Write();

    delete t_match_eff_phi;
  }

  delete h_pt_N;
  delete h_pt_D;
  delete h_th_N;
  delete h_th_D;
  delete h_phi_N;
  delete h_phi_D;

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
