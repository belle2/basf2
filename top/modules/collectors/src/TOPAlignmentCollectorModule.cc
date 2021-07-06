/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPAlignmentCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>

// framework aux
#include <framework/logging/Logger.h>

// root
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TRandom.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPAlignmentCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPAlignmentCollectorModule::TOPAlignmentCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for geometrical alignment of a TOP module with dimuons or Bhabhas. Iterative alignment procedure (NIMA 876 (2017) 260-264) is run here, algorithm just collects the results.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("targetModule", m_targetMid,
             "Module to be aligned. Must be 1 <= Mid <= 16.");
    addParam("maxFails", m_maxFails,
             "Maximum number of consecutive failed iterations before resetting the procedure", 100);
    addParam("sample", m_sample,
             "sample type: one of dimuon or bhabha", std::string("dimuon"));
    addParam("deltaEcms", m_deltaEcms,
             "c.m.s energy window (half size) if sample is dimuon or bhabha", 0.1);
    addParam("dr", m_dr, "cut on POCA in r", 1.0);
    addParam("dz", m_dz, "cut on POCA in abs(z)", 2.0);
    addParam("minZ", m_minZ, "minimal local z of extrapolated hit", -130.0);
    addParam("maxZ", m_maxZ, "maximal local z of extrapolated hit", 130.0);
    addParam("stepPosition", m_stepPosition, "step size for translations", 1.0);
    addParam("stepAngle", m_stepAngle, "step size for rotations", 0.01);
    addParam("stepTime", m_stepTime, "step size for t0", 0.05);
    std::string names;
    auto align = ModuleAlignment();
    for (const auto& parName : align.getParameterNames()) names += parName + ", ";
    names.pop_back();
    names.pop_back();
    addParam("parInit", m_parInit,
             "initial parameter values in the order [" + names + "]. "
             "If list is too short, the missing ones are set to 0.", m_parInit);
    auto parFixed = m_parFixed;
    addParam("parFixed", m_parFixed, "list of names of parameters to be fixed. "
             "Valid names are: " + names, parFixed);

  }


  void TOPAlignmentCollectorModule::prepare()
  {
    // input collections

    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isRequired();

    // check if target module ID is valid

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    if (!geo->isModuleIDValid(m_targetMid)) {
      B2FATAL("Target module ID = " << m_targetMid << " is invalid. Exiting...");
    }

    // set track selector

    if (m_sample == "dimuon" or m_sample == "bhabha") {
      m_selector = TrackSelector(m_sample);
      m_selector.setDeltaEcms(m_deltaEcms);
      m_selector.setCutOnPOCA(m_dr, m_dz);
      m_selector.setCutOnLocalZ(m_minZ, m_maxZ);
    } else {
      B2ERROR("Invalid sample type '" << m_sample << "'");
    }

    // set alignment objects

    for (int set = 0; set < c_numSets; set++) {
      auto align = ModuleAlignment();
      align.setModuleID(m_targetMid);
      align.setSteps(m_stepPosition, m_stepAngle, m_stepTime);
      align.setParameters(m_parInit);
      for (const auto& parName : m_parFixed) {
        align.fixParameter(parName);
      }
      m_align.push_back(align);
      m_countFails.push_back(0);
    }

    // create and register output histograms and ntuples

    int numModules = geo->getNumModules();
    auto h1 = new TH2F("tracks_per_slot", "Number of tracks per slot and sample",
                       numModules, 0.5, numModules + 0.5, c_numSets, 0, c_numSets);
    h1->SetXTitle("slot number");
    h1->SetYTitle("sample number");
    registerObject<TH2F>("tracks_per_slot", h1);

    for (int slot = 1; slot <= numModules; slot++) {
      std::string slotName = "_s" + to_string(slot);
      std::string slotTitle = "(slot " + to_string(slot) + ")";

      std::string hname = "local_z" + slotName;
      std::string title = "Distribution of tracks along bar " + slotTitle;
      auto h2 = new TH1F(hname.c_str(), title.c_str(), 100, -150.0, 150.0);
      h2->SetXTitle("local z [cm]");
      registerObject<TH1F>(hname, h2);

      hname = "cth_vs_p" + slotName;
      title = "Track momentum " + slotTitle;
      auto h3 = new TH2F(hname.c_str(), title.c_str(), 100, 0.0, 7.0, 100, -1.0, 1.0);
      h3->SetXTitle("p [GeV/c]");
      h3->SetYTitle("cos #theta");
      registerObject<TH2F>(hname, h3);

      hname = "poca_xy" + slotName;
      title = "Track POCA in x-y " + slotTitle;
      auto h4 = new TH2F(hname.c_str(), title.c_str(), 100, -m_dr, m_dr, 100, -m_dr, m_dr);
      h4->SetXTitle("x [cm]");
      h4->SetYTitle("y [cm]");
      registerObject<TH2F>(hname, h4);

      hname = "poca_z" + slotName;
      title = "Track POCA in z " + slotTitle;
      auto h5 = new TH1F(hname.c_str(), title.c_str(), 100, -m_dz, m_dz);
      h5->SetXTitle("z [cm]");
      registerObject<TH1F>(hname, h5);

      hname = "Ecms" + slotName;
      title = "Track c.m.s. energy " + slotTitle;
      auto h6 = new TH1F(hname.c_str(), title.c_str(), 100, 5.1, 5.4);
      h6->SetXTitle("E_{cms} [GeV]");
      registerObject<TH1F>(hname, h6);

      hname = "charge" + slotName;
      title = "Charge of track " + slotTitle;
      auto h7 = new TH1F(hname.c_str(), title.c_str(), 3, -1.5, 1.5);
      h7->SetXTitle("charge");
      registerObject<TH1F>(hname, h7);

      hname = "timeHits" + slotName;
      title = "Photon time distribution " + slotTitle;
      auto h8 = new TH2F(hname.c_str(), title.c_str(), 512, 0, 512, 200, 0, 50);
      h8->SetXTitle("channel number");
      h8->SetYTitle("time [ns]");
      registerObject<TH2F>(hname, h8);

      hname = "numPhot" + slotName;
      title = "Number of photons " + slotTitle;
      auto h9 = new TH1F(hname.c_str(), title.c_str(), 100, 0, 100);
      h9->SetXTitle("photons per track");
      registerObject<TH1F>(hname, h9);
    }

    for (int set = 0; set < c_numSets; set++) {
      std::string name = "alignTree" + to_string(set);
      m_treeNames.push_back(name);
      auto alignTree = new TTree(name.c_str(), "TOP alignment results");
      alignTree->Branch("ModuleId", &m_targetMid);
      alignTree->Branch("iter", &m_iter);
      alignTree->Branch("ntrk", &m_ntrk);
      alignTree->Branch("errorCode", &m_errorCode);
      alignTree->Branch("iterPars", &m_vAlignPars);
      alignTree->Branch("iterParsErr", &m_vAlignParsErr);
      alignTree->Branch("valid", &m_valid);
      alignTree->Branch("numPhot", &m_numPhot);
      alignTree->Branch("x", &m_x);
      alignTree->Branch("y", &m_y);
      alignTree->Branch("z", &m_z);
      alignTree->Branch("p", &m_p);
      alignTree->Branch("theta", &m_theta);
      alignTree->Branch("phi", &m_phi);
      alignTree->Branch("r_poca", &m_pocaR);
      alignTree->Branch("z_poca", &m_pocaZ);
      alignTree->Branch("x_poca", &m_pocaX);
      alignTree->Branch("y_poca", &m_pocaY);
      alignTree->Branch("Ecms", &m_cmsE);
      alignTree->Branch("charge", &m_charge);
      alignTree->Branch("PDG", &m_PDG);
      registerObject<TTree>(name, alignTree);
    }

  }


  void TOPAlignmentCollectorModule::collect()
  {
    // bunch must be reconstructed

    if (not m_recBunch.isValid()) return;
    if (not m_recBunch->isReconstructed()) return;

    // track-by-track iterations

    for (const auto& track : m_tracks) {

      // construct TOPTrack from mdst track
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      // skip if track not hitting target module
      if (trk.getModuleID() != m_targetMid) continue;

      // track selection
      if (not m_selector.isSelected(trk)) continue;

      // generate sub-sample number
      int sub = gRandom->Integer(c_numSets);
      auto& align = m_align[sub];
      auto& countFails = m_countFails[sub];
      const auto& name = m_treeNames[sub];
      auto h1 = getObjectPtr<TH2F>("tracks_per_slot");
      h1->Fill(trk.getModuleID(), sub);

      // do an iteration
      int err = align.iterate(trk, m_selector.getChargedStable());
      m_iter++;

      // check number of consecutive failures, and in case reset
      if (err == 0) {
        countFails = 0;
      } else if (countFails <= m_maxFails) {
        countFails++;
      } else {
        B2INFO("Reached maximum allowed number of failed iterations. "
               "Resetting TOPalign object of set = " << sub << " at iter = " << m_iter);
        align.reset();
        countFails = 0;
      }

      // get new parameter values and estimated errors
      m_vAlignPars = align.getParameters();
      m_vAlignParsErr = align.getErrors();
      m_ntrk = align.getNumUsedTracks();
      m_errorCode = err;
      m_valid = align.isValid();
      m_numPhot = align.getNumOfPhotons();

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
      auto alignTree = getObjectPtr<TTree>(name);
      alignTree->Fill();

      // fill control histograms
      std::string slotName = "_s" + to_string(m_targetMid);
      auto h2 = getObjectPtr<TH1F>("local_z" + slotName);
      h2->Fill(m_z);
      auto h3 = getObjectPtr<TH2F>("cth_vs_p" + slotName);
      h3->Fill(m_p, cos(m_theta));
      auto h4 = getObjectPtr<TH2F>("poca_xy" + slotName);
      h4->Fill(m_pocaX, m_pocaY);
      auto h5 = getObjectPtr<TH1F>("poca_z" + slotName);
      h5->Fill(m_pocaZ);
      auto h6 = getObjectPtr<TH1F>("Ecms" + slotName);
      h6->Fill(m_cmsE);
      auto h7 = getObjectPtr<TH1F>("charge" + slotName);
      h7->Fill(m_charge);
      auto h8 = getObjectPtr<TH2F>("timeHits" + slotName);
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getModuleID() != m_targetMid) continue;
        h8->Fill(digit.getChannel(), digit.getTime());
      }
      auto h9 = getObjectPtr<TH1F>("numPhot" + slotName);
      h9->Fill(m_numPhot);

    } // tracks

  }

} // Belle2
