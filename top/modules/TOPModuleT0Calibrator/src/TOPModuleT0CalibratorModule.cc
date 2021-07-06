/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPModuleT0Calibrator/TOPModuleT0CalibratorModule.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPModuleT0Calibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPModuleT0CalibratorModule::TOPModuleT0CalibratorModule() : Module()

  {
    // set module description
    setDescription("Module T0 calibration with dimuons or bhabhas. "
                   "Useful when the geometrical alignment is fine.");
    //    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("numBins", m_numBins, "number of bins of the search region", 200);
    addParam("timeRange", m_timeRange,
             "time range in which to search for the minimum [ns]", 10.0);
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
    addParam("outputFileName", m_outFileName,
             "Root output file name containing calibration results. "
             "File name can include *'s; "
             "they will be replaced with a run number from the first input file",
             std::string("moduleT0_r*.root"));
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("rough"));

  }

  void TOPModuleT0CalibratorModule::initialize()
  {
    // input collections
    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isOptional();

    // toggle has changed status to prevent warning in beginRun for the first IOV
    m_moduleT0.hasChanged();

    // Parse PDF option
    if (m_pdfOption == "rough") {
      m_PDFOption = PDFConstructor::c_Rough;
    } else if (m_pdfOption == "fine") {
      m_PDFOption = PDFConstructor::c_Fine;
    } else if (m_pdfOption == "optimal") {
      m_PDFOption = PDFConstructor::c_Optimal;
    } else {
      B2ERROR("TOPPDFDebuggerModule: unknown PDF option '" << m_pdfOption << "'");
    }

    // set track selector
    m_selector = TrackSelector(m_sample);
    m_selector.setMinMomentum(m_minMomentum);
    m_selector.setDeltaEcms(m_deltaEcms);
    m_selector.setCutOnPOCA(m_dr, m_dz);
    m_selector.setCutOnLocalZ(m_minZ, m_maxZ);

    // Chi2 minimum finders
    double tmin = -m_timeRange / 2;
    double tmax =  m_timeRange / 2;
    for (unsigned i = 0; i < 2; i++) {
      for (unsigned m = 0; m < c_numModules; m++) {
        m_finders[i][m] = Chi2MinimumFinder1D(m_numBins, tmin, tmax);
      }
    }

    // if file name includes *'s replace them with a run number
    auto pos = m_outFileName.find("*");
    if (pos != std::string::npos) {
      StoreObjPtr<EventMetaData> evtMetaData;
      auto run = std::to_string(evtMetaData->getRun());
      while (run.size() < 5) run = "0" + run;
      while (pos != std::string::npos) {
        m_outFileName.replace(pos, 1, run);
        pos = m_outFileName.find("*");
      }
    }

    // open root file for ntuple and histogram output
    m_file = TFile::Open(m_outFileName.c_str(), "RECREATE");
    if (not m_file) {
      B2ERROR("Cannot open output file '" << m_outFileName << "'");
      return;
    }

    // histograms
    m_hits1D = TH1F("numHits", "Number of photons per slot",
                    c_numModules, 0.5, c_numModules + 0.5);
    m_hits1D.SetXTitle("slot number");
    m_hits1D.SetYTitle("hits per slot");

    m_hits2D = TH2F("timeHits", "Photon times vs. boardstacks",
                    c_numModules * 4, 0.5, c_numModules + 0.5, 200, 0.0, 20.0);
    m_hits2D.SetXTitle("slot number");
    m_hits2D.SetYTitle("time [ns]");

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

  void TOPModuleT0CalibratorModule::beginRun()
  {
    if (m_moduleT0.hasChanged()) {
      StoreObjPtr<EventMetaData> evtMetaData;
      B2WARNING("ModuleT0 payload has changed. Calibration results may not be correct."
                << LogVar("run", evtMetaData->getRun()));
    }
  }

  void TOPModuleT0CalibratorModule::event()
  {
    /* check bunch reconstruction status and run alignment:
       - if object exists and bunch is found (collision data w/ bunch finder in the path)
       - if object doesn't exist (cosmic data and other cases w/o bunch finder)
    */

    if (m_recBunch.isValid()) {
      if (not m_recBunch->isReconstructed()) return;
    }

    TOPRecoManager::setDefaultTimeWindow();
    double timeMin = TOPRecoManager::getMinTime();
    double timeMax = TOPRecoManager::getMaxTime();

    // running offset must not be subtracted in TOPDigits: issue an error if it is

    if (isRunningOffsetSubtracted()) {
      B2ERROR("Running offset subtracted in TOPDigits: module T0 will not be correct");
    }

    // loop over reconstructed tracks, make a selection and accumulate log likelihoods

    for (const auto& track : m_tracks) {

      // track selection
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      if (not m_selector.isSelected(trk)) continue;

      // construct PDF
      PDFConstructor pdfConstructor(trk, m_selector.getChargedStable(), m_PDFOption);
      if (not pdfConstructor.isValid()) continue;

      // minimization procedure: accumulate
      const unsigned module = trk.getModuleID() - 1;
      if (module >= c_numModules) continue;
      int sub = gRandom->Integer(2); // generate sub-sample number
      auto& finder = m_finders[sub][module];
      const auto& binCenters = finder.getBinCenters();
      for (unsigned ibin = 0; ibin < binCenters.size(); ibin++) {
        double t0 = binCenters[ibin];
        finder.add(ibin, -2 * pdfConstructor.getLogL(t0, m_sigmaSmear).logL);
      }

      // fill histograms of hits
      m_numPhotons = 0;
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getModuleID() != trk.getModuleID()) continue;
        if (digit.getTime() < timeMin) continue;
        if (digit.getTime() > timeMax) continue;
        m_numPhotons++;
        m_hits1D.Fill(digit.getModuleID());
        int bs = digit.getBoardstackNumber();
        m_hits2D.Fill((digit.getModuleID() * 4 + bs - 1.5) / 4.0 , digit.getTime());
      }

      // fill output tree
      m_moduleID = trk.getModuleID();
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
      m_tree->Fill();
    }

  }


  void TOPModuleT0CalibratorModule::terminate()
  {
    // determine scaling factor for errors from two statistically independent results

    TH1F h_pulls("pulls", "Pulls of the two statistically independent results",
                 200, -15.0, 15.0);
    h_pulls.SetXTitle("pulls");
    for (unsigned module = 0; module < c_numModules; module++) {
      std::vector<double> pos, err;
      for (int i = 0; i < 2; i++) {
        const auto& minimum = m_finders[i][module].getMinimum();
        if (not minimum.valid) continue;
        pos.push_back(minimum.position);
        err.push_back(minimum.error);
      }
      if (pos.size() < 2) continue;
      double pull = (pos[0] - pos[1]) / sqrt(err[0] * err[0] + err[1] * err[1]);
      h_pulls.Fill(pull);
    }
    h_pulls.Write();
    double scaleError = h_pulls.GetRMS();

    // merge two statistically independent finders and store results into histograms

    TH1F h_relModuleT0("relModuleT0", "Module T0 relative to calibration",
                       c_numModules, 0.5, c_numModules + 0.5);
    h_relModuleT0.SetXTitle("slot number");
    h_relModuleT0.SetYTitle("module T0 residual [ns]");

    for (unsigned module = 0; module < c_numModules; module++) {
      auto& finder = m_finders[0][module].add(m_finders[1][module]);
      const auto& minimum = finder.getMinimum();
      if (minimum.valid) {
        h_relModuleT0.SetBinContent(module + 1, minimum.position);
        h_relModuleT0.SetBinError(module + 1, minimum.error * scaleError);
      }
      std::string slotNum = to_string(module + 1);
      auto h = finder.getHistogram("chi2_slot_" + slotNum, "slot " + slotNum);
      h.Write();
    }
    h_relModuleT0.Write();

    // absolute module T0
    TH1F h_moduleT0("moduleT0", "Module T0",
                    c_numModules, 0.5, c_numModules + 0.5);
    h_moduleT0.SetXTitle("slot number");
    h_moduleT0.SetYTitle("module T0 [ns]");

    double average = 0; // average to be subtracted
    int num = 0;
    for (int slot = 1; slot <= c_numModules; slot++) {
      if (h_relModuleT0.GetBinError(slot) > 0) {
        double T0 = 0;
        if (m_moduleT0->isCalibrated(slot)) T0 = m_moduleT0->getT0(slot);
        average += h_relModuleT0.GetBinContent(slot) + T0;
        num++;
      }
    }
    if (num > 0) average /= num;

    for (int slot = 1; slot <= c_numModules; slot++) {
      double T0 = 0;
      if (m_moduleT0->isCalibrated(slot)) T0 = m_moduleT0->getT0(slot);
      double t0 = h_relModuleT0.GetBinContent(slot) + T0 - average;
      double err = h_relModuleT0.GetBinError(slot);
      h_moduleT0.SetBinContent(slot, t0);
      h_moduleT0.SetBinError(slot, err);
    }
    h_moduleT0.Write();

    // write other histograms and ntuple; close the file

    m_hits1D.Write();
    m_hits2D.Write();
    m_tree->Write();
    m_file->Close();

    B2RESULT(num << "/16 calibrated. Results available in " << m_outFileName);
  }


  bool TOPModuleT0CalibratorModule::isRunningOffsetSubtracted()
  {
    for (const auto& digit : m_digits) {
      if (digit.hasStatus(TOPDigit::c_BunchOffsetSubtracted)) return true;
    }
    return false;
  }

} // end Belle2 namespace

