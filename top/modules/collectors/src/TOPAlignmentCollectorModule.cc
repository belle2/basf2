/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/collectors/TOPAlignmentCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPconfigure.h>
#include <top/reconstruction/TOPtrack.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// root
#include <TTree.h>
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
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per module", 0.0);
    addParam("scaleN0", m_scaleN0,
             "Scale factor for figure-of-merit N0", 1.0);
    addParam("targetModule", m_targetMid,
             "Module to be aligned. Must be 1 <= Mid <= 16.");
    addParam("maxFails", m_maxFails,
             "Maximum number of consecutive failed iterations before resetting the procedure", 100);
    addParam("sample", m_sample,
             "sample type: one of dimuon or bhabha", std::string("dimuon"));
    addParam("deltaEcms", m_deltaEcms,
             "c.m.s energy window (half size) if sample is dimuon or bhabha", 0.1);
    addParam("dr", m_dr, "cut on POCA in r", 2.0);
    addParam("dz", m_dz, "cut on POCA in abs(z)", 4.0);
    addParam("minZ", m_minZ, "minimal local z of extrapolated hit", -130.0);
    addParam("maxZ", m_maxZ, "maximal local z of extrapolated hit", 130.0);
    addParam("stepPosition", m_stepPosition, "step size for translations", 1.0);
    addParam("stepAngle", m_stepAngle, "step size for rotations", 0.01);
    addParam("stepTime", m_stepTime, "step size for t0", 0.05);
    addParam("stepRefind", m_stepRefind,
             "step size for scaling of refractive index (dn/n)", 0.005);
    addParam("gridSize", m_gridSize,
             "size of a 2D grid for time-of-propagation averaging in analytic PDF: "
             "[number of emission points along track, number of Cerenkov angles]. "
             "No grid used if list is empty.", m_gridSize);
    std::string names;
    auto align = TOPalign();
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
      auto align = TOPalign();
      align.setModuleID(m_targetMid);
      align.setSteps(m_stepPosition, m_stepAngle, m_stepTime, m_stepRefind);
      if (m_gridSize.size() == 2) {
        align.setGrid(m_gridSize[0], m_gridSize[1]);
        B2INFO("TOPAligner: grid for time-of-propagation averaging is set");
      }
      align.setParameters(m_parInit);
      for (const auto& parName : m_parFixed) {
        align.fixParameter(parName);
      }
      m_align.push_back(align);
      m_countFails.push_back(0);
    }

    // configure detector in reconstruction code

    TOPconfigure config;

    // create and register output histograms and ntuples

    int numModules = geo->getNumModules();
    auto h = new TH2F("tracks_per_slot", "tracks per slot and sample",
                      numModules, 0.5, numModules + 0.5, c_numSets, 0, c_numSets);
    h->SetXTitle("slot number");
    h->SetYTitle("sample number");
    registerObject<TH2F>("tracks_per_slot", h);

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

    // add photons

    TOPalign::clearData();

    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() == TOPDigit::c_Good)
        TOPalign::addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                          digit.getTimeError());
    }

    TOPalign::setPhotonYields(m_minBkgPerBar, m_scaleN0);

    // track-by-track iterations

    for (const auto& track : m_tracks) {

      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
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
               "Resetting TOPalign object");
        align.reset();
        countFails = 0;
      }

      // get new parameter values and estimated errors
      m_vAlignPars = align.getParameters();
      m_vAlignParsErr = align.getErrors();
      m_ntrk = align.getNumUsedTracks();
      m_errorCode = err;
      m_valid = align.isValid();

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
      m_PDG = trk.getPDGcode();

      // fill output tree
      auto alignTree = getObjectPtr<TTree>(name);
      alignTree->Fill();

    } // tracks

  }

} // Belle2
