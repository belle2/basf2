/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPCommonT0Calibrator/TOPCommonT0CalibratorModule.h>
#include <top/geometry/TOPGeometryPar.h>
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

  REG_MODULE(TOPCommonT0Calibrator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCommonT0CalibratorModule::TOPCommonT0CalibratorModule() : Module()

  {
    // set module description
    setDescription("Common T0 calibration with dimuons or bhabhas.");
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
             std::string("commonT0_r*.root"));
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("rough"));

  }


  void TOPCommonT0CalibratorModule::initialize()
  {
    // input collections
    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isOptional();

    // bunch separation in time

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_bunchTimeSep = geo->getNominalTDC().getSyncTimeBase() / 24;

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
    for (unsigned i = 0; i < c_numSets; i++) {
      m_finders[i] = Chi2MinimumFinder1D(m_numBins, tmin, tmax);
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

    // control histograms
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


  void TOPCommonT0CalibratorModule::event()
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
      B2ERROR("Running offset subtracted in TOPDigits: common T0 will not be correct");
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
      int sub = gRandom->Integer(c_numSets); // generate sub-sample number
      auto& finder = m_finders[sub];
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


  void TOPCommonT0CalibratorModule::terminate()
  {

    // determine scaling factor for errors from statistically independent results

    TH1F h_pulls("pulls", "Pulls of statistically independent results",
                 200, -15.0, 15.0);
    h_pulls.SetXTitle("pulls");
    std::vector<double> pos, err;
    for (int i = 0; i < c_numSets; i++) {
      const auto& minimum = m_finders[i].getMinimum();
      if (not minimum.valid) continue;
      pos.push_back(minimum.position);
      err.push_back(minimum.error);
    }
    for (unsigned i = 0; i < pos.size(); i++) {
      for (unsigned j = i + 1; j < pos.size(); j++) {
        double pull = (pos[i] - pos[j]) / sqrt(err[i] * err[i] + err[j] * err[j]);
        h_pulls.Fill(pull);
      }
    }
    h_pulls.Write();
    double scaleError = 1;
    if (h_pulls.GetEntries() > 1) scaleError = h_pulls.GetRMS();

    // merge statistically independent finders and store results into histograms

    auto finder = m_finders[0];
    for (int i = 1; i < c_numSets; i++) {
      finder.add(m_finders[i]);
    }

    TH1F h_relCommonT0("relCommonT0", "relative common T0", 1, 0, 1);
    h_relCommonT0.SetYTitle("common T0 residual [ns]");
    TH1F h_commonT0("commonT0", "Common T0", 1, 0, 1);
    h_commonT0.SetYTitle("common T0 [ns]");

    const auto& minimum = finder.getMinimum();
    auto h = finder.getHistogram("chi2", "chi2");
    h.Write();
    if (minimum.valid) {
      h_relCommonT0.SetBinContent(1, minimum.position);
      h_relCommonT0.SetBinError(1, minimum.error * scaleError);
      double T0 = minimum.position;
      if (m_commonT0->isCalibrated()) T0 += m_commonT0->getT0();
      T0 -= round(T0 / m_bunchTimeSep) * m_bunchTimeSep; // wrap around
      h_commonT0.SetBinContent(1, T0);
      h_commonT0.SetBinError(1, minimum.error * scaleError);
    }
    h_relCommonT0.Write();
    h_commonT0.Write();

    // write other histograms and ntuple; close the file

    m_hits1D.Write();
    m_hits2D.Write();
    m_tree->Write();
    m_file->Close();

    B2RESULT("Results available in " << m_outFileName);
  }

  bool TOPCommonT0CalibratorModule::isRunningOffsetSubtracted()
  {
    for (const auto& digit : m_digits) {
      if (digit.hasStatus(TOPDigit::c_BunchOffsetSubtracted)) return true;
    }
    return false;
  }

} // end Belle2 namespace

