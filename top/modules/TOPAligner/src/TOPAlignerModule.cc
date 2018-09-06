/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPAligner/TOPAlignerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPalign.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// analysis
#include <analysis/utility/PCmsLabTransform.h>

// root
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
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per module", 0.0);
    addParam("scaleN0", m_scaleN0,
             "Scale factor for figure-of-merit N0", 1.0);
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
    addParam("dr", m_dr, "cut on POCA in r", 1.0);
    addParam("dz", m_dz, "cut on POCA in z", 5.0);
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
    addParam("gridSize", m_gridSize,
             "size of a 2D grid for time-of-propagation averaging in analytic PDF: "
             "[number of emission points along track, number of Cerenkov angles]. "
             "No grid used if list is empty.", m_gridSize);
    std::string names;
    for (const auto& parName : m_align.getParameterNames()) names += parName + ", ";
    names.pop_back();
    names.pop_back();
    addParam("parInit", m_parInit,
             "initial parameter values in the order [" + names + "]. "
             "If list is too short, the missing ones are set to 0.", m_parInit);
    addParam("parFixed", m_parFixed, "list of names of parameters to be fixed. "
             "Valid names are: " + names, m_parFixed);

  }

  TOPAlignerModule::~TOPAlignerModule()
  {
  }

  void TOPAlignerModule::initialize()
  {

    // check if target module ID is valid

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    if (!geo->isModuleIDValid(m_targetMid))
      B2FATAL("Target module ID = " << m_targetMid << " is invalid. Exiting...");

    // check if sample type is valid

    if (!(m_sample == "dimuon" or m_sample == "bhabha" or m_sample == "cosmics")) {
      B2FATAL("Invalid sample type " << m_sample << ". Exiting...");
    }
    if (m_sample == "bhabha") m_chargedStable = Const::electron;

    // create alignment object for a given target module

    m_align = TOPalign(m_targetMid, m_stepPosition, m_stepAngle, m_stepTime);
    if (m_gridSize.size() == 2) {
      m_align.setGrid(m_gridSize[0], m_gridSize[1]);
      B2INFO("TOPAligner: grid for time-of-propagation averaging is set");
    }
    m_align.setParameters(m_parInit);
    for (const auto& parName : m_parFixed) {
      m_align.fixParameter(parName);
    }

    // configure detector in reconstruction code

    TOPconfigure config;

    // input

    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();

    // set counter for failed iterations:

    m_countFails = 0;

    // output file

    m_file = new TFile(m_outFileName.c_str(), "RECREATE");
    if (m_file->IsZombie()) {
      B2FATAL("Couldn't open file '" << m_outFileName << "' for writing!");
      return;
    }

    m_alignTree = new TTree("alignTree", "TOP alignment results");

    m_alignTree->Branch("ModuleId", &m_targetMid);
    m_alignTree->Branch("ntrk", &m_ntrk);
    m_alignTree->Branch("errorCode", &m_errorCode);
    m_alignTree->Branch("iterPars", &m_vAlignPars);
    m_alignTree->Branch("iterParsErr", &m_vAlignParsErr);
    m_alignTree->Branch("valid", &m_valid);
    m_alignTree->Branch("x", &m_x);
    m_alignTree->Branch("y", &m_y);
    m_alignTree->Branch("z", &m_z);
    m_alignTree->Branch("p", &m_p);
    m_alignTree->Branch("theta", &m_theta);
    m_alignTree->Branch("phi", &m_phi);
    m_alignTree->Branch("r_poca", &m_pocaR);
    m_alignTree->Branch("z_poca", &m_pocaZ);
    m_alignTree->Branch("Ecms", &m_cmsE);
    m_alignTree->Branch("charge", &m_charge);
    m_alignTree->Branch("PDG", &m_PDG);

  }

  void TOPAlignerModule::beginRun()
  {
  }

  void TOPAlignerModule::event()
  {

    // add photons

    TOPalign::clearData();

    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good)
        TOPalign::addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                          digit.getTimeError());
    }

    TOPalign::setPhotonYields(m_minBkgPerBar, m_scaleN0);

    // track-by-track iterations

    for (const auto& track : m_tracks) {

      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // skip if track not hitting target module
      if (trk.getModuleID() != m_targetMid) continue;

      // track selection
      if (!selectTrack(trk)) continue;

      // do an iteration
      int err = m_align.iterate(trk, m_chargedStable);

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
      m_ntrk = m_align.getNumTracks();
      m_errorCode = err;
      m_valid = m_align.isValid();

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
      B2INFO(resMsg);

    }

  }


  void TOPAlignerModule::endRun()
  {
  }

  void TOPAlignerModule::terminate()
  {

    m_file->cd();
    m_alignTree->Write();

    int npar = m_align.getParameters().size();
    const auto& errMatrix = m_align.getErrorMatrix();
    TH2F h1("errMatrix", "error matrix", npar, 0, npar, npar, 0, npar);
    for (int i = 0; i < npar; i++) {
      for (int k = 0; k < npar; k++) {
        h1.SetBinContent(i + 1, k + 1, errMatrix[i + npar * k]);
      }
    }
    h1.Write();

    TH2F h2("corMatrix", "correlation matrix", npar, 0, npar, npar, 0, npar);
    std::vector<double> diag;
    for (int i = 0; i < npar; i++) {
      double d = errMatrix[i * (1 + npar)];
      if (d != 0) d = 1.0 / sqrt(d);
      diag.push_back(d);
    }
    for (int i = 0; i < npar; i++) {
      for (int k = 0; k < npar; k++) {
        h2.SetBinContent(i + 1, k + 1, diag[i] * diag[k] * errMatrix[i + npar * k]);
      }
    }
    h2.Write();

    m_file->Close();

  }

  bool TOPAlignerModule::selectTrack(const TOP::TOPtrack& trk)
  {

    const auto* fit = trk.getTrack()->getTrackFitResultWithClosestMass(m_chargedStable);
    auto pocaPosition = fit->getPosition();
    m_pocaR = pocaPosition.Perp();
    m_pocaZ = pocaPosition.Z();
    if (m_pocaR > m_dr) return false;
    if (fabs(m_pocaZ) > m_dz) return false;

    auto pocaMomentum = fit->getMomentum();

    if (m_sample == "cosmics") {
      if (pocaMomentum.Mag() < m_minMomentum) return false;
    } else {
      TLorentzVector lorentzLab;
      lorentzLab.SetXYZM(pocaMomentum.X(), pocaMomentum.Y(), pocaMomentum.Z(),
                         m_chargedStable.getMass());
      PCmsLabTransform T;
      auto lorentzCms = T.labToCms(lorentzLab);
      m_cmsE = lorentzCms.Energy();
      double dE = m_cmsE - T.getCMSEnergy() / 2;
      if (fabs(dE) > m_deltaEcms) return false;
    }

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    const auto& module = geo->getModule(m_targetMid);
    auto position = module.pointToLocal(trk.getPosition());
    auto momentum = module.momentumToLocal(trk.getMomentum());
    m_x = position.X();
    m_y = position.Y();
    m_z = position.Z();
    m_p = momentum.Mag();
    m_theta = momentum.Theta();
    m_phi = momentum.Phi();
    m_charge = trk.getCharge();
    m_PDG = trk.getPDGcode();

    if (m_z < m_minZ or m_z > m_maxZ) return false;

    return true;
  }

} // end Belle2 namespace

