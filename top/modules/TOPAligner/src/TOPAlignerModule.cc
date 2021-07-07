/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPAligner/TOPAlignerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <framework/logging/Logger.h>

// root
#include <TH1F.h>
#include <TH2F.h>


using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPAligner)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPAlignerModule::TOPAlignerModule() : Module()

  {
    // set module description
    setDescription("Alignment of TOP");
    //    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("targetModule", m_targetMid,
             "Module to be aligned. Must be 1 <= Mid <= 16.", 1);
    addParam("maxFails", m_maxFails,
             "Maximum number of consecutive failed iterations before resetting the procedure", 100);
    addParam("sample", m_sample,
             "sample type: one of dimuon, bhabha or cosmics", std::string("dimuon"));
    addParam("minMomentum", m_minMomentum,
             "minimal track momentum if sample is cosmics", 1.0);
    addParam("deltaEcms", m_deltaEcms,
             "c.m.s energy window (half size) if sample is dimuon or bhabha", 0.1);
    addParam("dr", m_dr, "cut on POCA in r", 2.0);
    addParam("dz", m_dz, "cut on POCA in abs(z)", 4.0);
    addParam("minZ", m_minZ,
             "minimal local z of extrapolated hit", -130.0);
    addParam("maxZ", m_maxZ,
             "maximal local z of extrapolated hit", 130.0);
    addParam("outFileName", m_outFileName,
             "Root output file name containing alignment results",
             std::string("TopAlignPars.root"));
    addParam("stepPosition", m_stepPosition, "step size for translations", 1.0);
    addParam("stepAngle", m_stepAngle, "step size for rotations", 0.01);
    addParam("stepTime", m_stepTime, "step size for t0", 0.05);
    std::string names;
    for (const auto& parName : m_align.getParameterNames()) names += parName + ", ";
    names.pop_back();
    names.pop_back();
    addParam("parInit", m_parInit,
             "initial parameter values in the order [" + names + "]. "
             "If list is too short, the missing ones are set to 0.", m_parInit);
    auto parFixed = m_parFixed;
    addParam("parFixed", m_parFixed, "list of names of parameters to be fixed. "
             "Valid names are: " + names, parFixed);

  }

  void TOPAlignerModule::initialize()
  {
    // check if target module ID is valid

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    if (not geo->isModuleIDValid(m_targetMid)) {
      B2ERROR("Target module ID = " << m_targetMid << " is invalid.");
    }

    // check if sample type is valid

    if (not(m_sample == "dimuon" or m_sample == "bhabha" or m_sample == "cosmics")) {
      B2ERROR("Invalid sample type '" << m_sample << "'");
    }
    if (m_sample == "bhabha") m_chargedStable = Const::electron;

    // set track selector

    m_selector = TrackSelector(m_sample);
    m_selector.setMinMomentum(m_minMomentum);
    m_selector.setDeltaEcms(m_deltaEcms);
    m_selector.setCutOnPOCA(m_dr, m_dz);
    m_selector.setCutOnLocalZ(m_minZ, m_maxZ);

    // set alignment object

    m_align.setModuleID(m_targetMid);
    m_align.setSteps(m_stepPosition, m_stepAngle, m_stepTime);
    m_align.setParameters(m_parInit);
    for (const auto& parName : m_parFixed) {
      m_align.fixParameter(parName);
    }

    // input

    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isOptional();

    // open output file

    m_file = TFile::Open(m_outFileName.c_str(), "RECREATE");
    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_outFileName << "' for writing!");
      return;
    }

    // create output tree

    m_alignTree = new TTree("alignTree", "TOP alignment results");
    m_alignTree->Branch("ModuleId", &m_targetMid);
    m_alignTree->Branch("iter", &m_iter);
    m_alignTree->Branch("ntrk", &m_ntrk);
    m_alignTree->Branch("errorCode", &m_errorCode);
    m_alignTree->Branch("iterPars", &m_vAlignPars);
    m_alignTree->Branch("iterParsErr", &m_vAlignParsErr);
    m_alignTree->Branch("valid", &m_valid);
    m_alignTree->Branch("numPhot", &m_numPhot);
    m_alignTree->Branch("x", &m_x);
    m_alignTree->Branch("y", &m_y);
    m_alignTree->Branch("z", &m_z);
    m_alignTree->Branch("p", &m_p);
    m_alignTree->Branch("theta", &m_theta);
    m_alignTree->Branch("phi", &m_phi);
    m_alignTree->Branch("r_poca", &m_pocaR);
    m_alignTree->Branch("z_poca", &m_pocaZ);
    m_alignTree->Branch("x_poca", &m_pocaX);
    m_alignTree->Branch("y_poca", &m_pocaY);
    m_alignTree->Branch("Ecms", &m_cmsE);
    m_alignTree->Branch("charge", &m_charge);
    m_alignTree->Branch("PDG", &m_PDG);

  }

  void TOPAlignerModule::event()
  {

    // check bunch reconstruction status and run alignment:
    // - if object exists and bunch is found (collision data w/ bunch finder in the path)
    // - if object doesn't exist (cosmic data and other cases w/o bunch finder)

    if (m_recBunch.isValid()) {
      if (not m_recBunch->isReconstructed()) return;
    }

    // track-by-track iterations

    for (const auto& track : m_tracks) {

      // construct TOPtrack from mdst track
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      // skip if track not hitting target module
      if (trk.getModuleID() != m_targetMid) continue;

      // track selection
      if (not m_selector.isSelected(trk)) continue;

      // do an iteration
      int err = m_align.iterate(trk, m_chargedStable);
      m_iter++;

      // check number of consecutive failures, and in case reset
      if (err == 0) {
        m_countFails = 0;
      } else if (m_countFails <= m_maxFails) {
        m_countFails++;
      } else {
        B2INFO("Reached maximum allowed number of failed iterations. "
               "Resetting TOPalign object");
        m_align.reset();
        m_countFails = 0;
      }

      // get new parameter values and estimated errors
      m_vAlignPars = m_align.getParameters();
      m_vAlignParsErr = m_align.getErrors();
      m_ntrk = m_align.getNumUsedTracks();
      m_errorCode = err;
      m_valid = m_align.isValid();
      m_numPhot = m_align.getNumOfPhotons();

      // set other ntuple variables
      const auto& localPosition = m_selector.getLocalPosition();
      m_x = localPosition.X();
      m_y = localPosition.Y();
      m_z = localPosition.Z();
      const auto& localMomentum = m_selector.getLocalMomentum();
      m_p = localMomentum.Mag();
      m_theta = localMomentum.Theta();
      m_phi = localMomentum.Phi();
      const auto& pocaPosition = m_selector.getPOCAPosition();
      m_pocaR = pocaPosition.Perp();
      m_pocaZ = pocaPosition.Z();
      m_pocaX = pocaPosition.X();
      m_pocaY = pocaPosition.Y();
      m_cmsE = m_selector.getCMSEnergy();
      m_charge = trk.getCharge();
      m_PDG = trk.getPDGCode();

      // fill output tree
      m_alignTree->Fill();

      // print info
      TString resMsg = "M= ";
      resMsg += m_align.getModuleID();
      resMsg += " ntr=";
      resMsg += m_ntrk;
      resMsg += " err=";
      resMsg += m_errorCode;
      resMsg += " v=";
      resMsg += m_align.isValid();
      for (auto par : m_vAlignPars) {
        resMsg += " ";
        resMsg += par;
      }
      B2DEBUG(100, resMsg);

    }

  }


  void TOPAlignerModule::terminate()
  {

    m_file->cd();
    m_alignTree->Write();

    TH1F valid("valid", "status valid", 16, 0.5, 16.5);
    valid.SetXTitle("slot ID");
    valid.SetBinContent(m_targetMid, m_valid);
    valid.Write();

    TH1F ntrk("ntrk", "number of tracks", 16, 0.5, 16.5);
    ntrk.SetXTitle("slot ID");
    ntrk.SetBinContent(m_targetMid, m_ntrk);
    ntrk.Write();

    std::string name, title;
    name = "results_slot" + to_string(m_targetMid);
    title = "alignment parameters, slot " + to_string(m_targetMid);
    int npar = m_align.getParams().size();
    TH1F h0(name.c_str(), title.c_str(), npar, 0, npar);
    const auto& par = m_align.getParams();
    const auto& err = m_align.getErrors();
    for (int i = 0; i < npar; i++) {
      h0.SetBinContent(i + 1, par[i]);
      h0.SetBinError(i + 1, err[i]);
    }
    h0.Write();

    name = "errMatrix_slot" + to_string(m_targetMid);
    title = "error matrix, slot " + to_string(m_targetMid);
    TH2F h1(name.c_str(), title.c_str(), npar, 0, npar, npar, 0, npar);
    const auto& errMatrix = m_align.getErrorMatrix();
    for (int i = 0; i < npar; i++) {
      for (int k = 0; k < npar; k++) {
        h1.SetBinContent(i + 1, k + 1, errMatrix[i][k]);
      }
    }
    h1.Write();

    name = "corMatrix_slot" + to_string(m_targetMid);
    title = "correlation matrix, slot " + to_string(m_targetMid);
    TH2F h2(name.c_str(), title.c_str(), npar, 0, npar, npar, 0, npar);
    std::vector<double> diag;
    for (int i = 0; i < npar; i++) {
      double d = errMatrix[i][i];
      if (d != 0) d = 1.0 / sqrt(d);
      diag.push_back(d);
    }
    for (int i = 0; i < npar; i++) {
      for (int k = 0; k < npar; k++) {
        h2.SetBinContent(i + 1, k + 1, diag[i] * diag[k] * errMatrix[i][k]);
      }
    }
    h2.Write();

    m_file->Close();

    if (m_valid) {
      B2RESULT("TOPAligner: slot = " << m_targetMid << ", status = successful, "
               << "iterations = " << m_iter << ", tracks used = " << m_ntrk);
    } else {
      B2RESULT("TOPAligner: slot = " << m_targetMid << ", status = failed, "
               << "error code = " << m_errorCode
               << ", iterations = " << m_iter << ", tracks used = " << m_ntrk);
    }
  }


} // end Belle2 namespace

