/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPChannelT0Calibrator/TOPChannelT0CalibratorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
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

// root
#include <TRandom.h>
#include <TH1F.h>


using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPChannelT0Calibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPChannelT0CalibratorModule::TOPChannelT0CalibratorModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Alternative channel T0 calibration with collision data. \n"
                   "Note: after this kind of calibration one cannot do the alignment.");
    //    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("numBins", m_numBins, "number of bins of the search region", 200);
    addParam("timeRange", m_timeRange,
             "time range in which to search  for the minimum [ns]", 10.0);
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per module", 0.0);
    addParam("scaleN0", m_scaleN0,
             "Scale factor for figure-of-merit N0", 1.0);
    addParam("sigmaSmear", m_sigmaSmear,
             "sigma in [ns] for additional smearing of PDF", 0.0);
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
             "Root output file name containing calibration results",
             std::string("channelT0alternative.root"));

  }

  TOPChannelT0CalibratorModule::~TOPChannelT0CalibratorModule()
  {
  }

  void TOPChannelT0CalibratorModule::initialize()
  {
    // input collections
    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isOptional();

    // Configure TOP detector for reconstruction
    TOPconfigure config;

    // set track selector
    m_selector = TrackSelector(m_sample);
    m_selector.setMinMomentum(m_minMomentum);
    m_selector.setDeltaEcms(m_deltaEcms);
    m_selector.setCutOnPOCA(m_dr, m_dz);
    m_selector.setCutOnLocalZ(m_minZ, m_maxZ);

    // minimum finders
    double tmin = -m_timeRange / 2;
    double tmax =  m_timeRange / 2;
    for (unsigned i = 0; i < 2; i++) {
      for (unsigned m = 0; m < c_numModules; m++) {
        for (unsigned c = 0; c < c_numChannels; c++) {
          m_finders[i][m][c] = Chi2MinimumFinder1D(m_numBins, tmin, tmax);
        }
      }
    }

    // open root file for ntuple and histogram output
    m_file = TFile::Open(m_outFileName.c_str(), "RECREATE");

    // create output tree

    m_tree = new TTree("tree", "Channel T0 calibration results");
    m_tree->Branch("slot", &m_moduleID);
    m_tree->Branch("numPhotons", &m_numPhotons);
    m_tree->Branch("x", &m_x);
    m_tree->Branch("y", &m_y);
    m_tree->Branch("z", &m_z);
    m_tree->Branch("p", &m_p);
    m_tree->Branch("theta", &m_theta);
    m_tree->Branch("phi", &m_phi);
    m_tree->Branch("r_poca", &m_pocaR);
    m_tree->Branch("z_poca", &m_pocaZ);
    m_tree->Branch("x_poca", &m_pocaX);
    m_tree->Branch("y_poca", &m_pocaY);
    m_tree->Branch("Ecms", &m_cmsE);
    m_tree->Branch("charge", &m_charge);
    m_tree->Branch("PDG", &m_PDG);

  }

  void TOPChannelT0CalibratorModule::beginRun()
  {
  }

  void TOPChannelT0CalibratorModule::event()
  {
    /* check bunch reconstruction status and run alignment:
       - if object exists and bunch is found (collision data w/ bunch finder in the path)
       - if object doesn't exist (cosmic data and other cases w/o bunch finder)
    */

    if (m_recBunch.isValid()) {
      if (!m_recBunch->isReconstructed()) return;
    }

    // create reconstruction object and set various options

    int Nhyp = 1;
    double mass = m_selector.getChargedStable().getMass();
    TOPreco reco(Nhyp, &mass, m_minBkgPerBar, m_scaleN0);
    reco.setPDFoption(TOPreco::c_Rough); // TODO: c_Fine?
    const auto& tdc = TOPGeometryPar::Instance()->getGeometry()->getNominalTDC();
    double timeMin = tdc.getTimeMin();
    double timeMax = tdc.getTimeMax();

    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();

    // add photon hits to reconstruction object

    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                     digit.getTimeError());
    }

    // loop over reconstructed tracks, make a selection and accumulate log likelihoods

    for (const auto& track : m_tracks) {

      // track selection
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      if (!m_selector.isSelected(trk)) continue;

      // run reconstruction
      reco.reconstruct(trk);
      if (reco.getFlag() != 1) continue; // track is not in the acceptance of TOP

      // minimization procedure: accumulate
      unsigned module = trk.getModuleID() - 1;
      if (module >= c_numModules) continue;
      int sub = gRandom->Integer(2); // generate sub-sample number
      auto& finders = m_finders[sub][module];
      const auto& binCenters = finders[0].getBinCenters();
      for (unsigned ibin = 0; ibin < binCenters.size(); ibin++) {
        double t0 = binCenters[ibin];
        std::vector<float> logL;
        logL.resize(c_numChannels, 0);
        reco.getLogL(t0, timeMin, timeMax, m_sigmaSmear, logL.data());
        for (unsigned channel = 0; channel < c_numChannels; channel++) {
          int pix = chMapper.getPixelID(channel) - 1;
          finders[channel].add(ibin, -2 * logL[pix]);
        }
      }

      // fill output tree
      m_moduleID = trk.getModuleID();
      m_numPhotons = reco.getNumOfPhotons();
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
      m_tree->Fill();
    }

  }


  void TOPChannelT0CalibratorModule::endRun()
  {
  }

  void TOPChannelT0CalibratorModule::terminate()
  {

    // determine scaling factor for errors from two statistically independent results

    TH1F h_pulls("pulls", "Pulls of the two statistically independent results",
                 200, -15.0, 15.0);
    h_pulls.SetXTitle("pulls");
    for (unsigned module = 0; module < c_numModules; module++) {
      for (unsigned channel = 0; channel < c_numChannels; channel++) {
        std::vector<double> pos, err;
        for (int i = 0; i < 2; i++) {
          const auto& minimum = m_finders[i][module][channel].getMinimum();
          if (not minimum.valid) continue;
          pos.push_back(minimum.position);
          err.push_back(minimum.error);
        }
        if (pos.size() < 2) continue;
        double pull = (pos[0] - pos[1]) / sqrt(err[0] * err[0] + err[1] * err[1]);
        h_pulls.Fill(pull);
      }
    }
    h_pulls.Write();
    double scaleError = h_pulls.GetRMS();

    // merge two statistically independent results and store into histograms

    for (unsigned module = 0; module < c_numModules; module++) {
      int moduleID = module + 1;

      std::string slotNum = std::to_string(moduleID);
      if (moduleID < 10) slotNum = "0" + slotNum;
      std::string name, title;

      name = "valid_slot" + slotNum;
      title = "status 'valid' for slot " + slotNum;
      TH1F h_valid(name.c_str(), title.c_str(), c_numChannels, 0, c_numChannels);
      h_valid.SetXTitle("channel number");
      h_valid.SetYTitle("minimization status (1 = OK)");

      name = "relT0_slot" + slotNum;
      title = "channel T0 for slot " + slotNum;
      TH1F h_relT0(name.c_str(), title.c_str(), c_numChannels, 0, c_numChannels);
      h_relT0.SetXTitle("channel number");
      h_relT0.SetYTitle("relative channel T0 [ns]");

      for (unsigned channel = 0; channel < c_numChannels; channel++) {
        auto& finder = m_finders[0][module][channel].add(m_finders[1][module][channel]);
        const auto& minimum = finder.getMinimum();
        h_valid.SetBinContent(channel + 1, minimum.valid);
        if (minimum.valid) {
          h_relT0.SetBinContent(channel + 1, minimum.position);
          h_relT0.SetBinError(channel + 1, minimum.error * scaleError);
        }
      }

      h_relT0.Write();
      h_valid.Write();

    }

    m_tree->Write();
    m_file->Close();
  }


} // end Belle2 namespace

