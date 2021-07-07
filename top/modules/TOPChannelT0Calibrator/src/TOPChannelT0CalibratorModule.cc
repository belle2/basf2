/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPChannelT0Calibrator/TOPChannelT0CalibratorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <TRandom.h>

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
    // set module description
    setDescription("Alternative channel T0 calibration with dimuons or bhabhas. "
                   "This module can also be used to check the calibration "
                   "(channel, module and common T0).\n"
                   "Note: after this kind of calibration one cannot do the alignment.");
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
             std::string("calibrationT0_r*.root"));
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("rough"));

  }

  void TOPChannelT0CalibratorModule::initialize()
  {
    // input collections
    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isOptional();

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
    for (unsigned module = 0; module < c_numModules; module++) {
      int moduleID = module + 1;

      std::string slotNum = std::to_string(moduleID);
      if (moduleID < 10) slotNum = "0" + slotNum;

      std::string name = "numHits_slot" + slotNum;
      std::string title = "Number of hits per channel for slot " + slotNum;
      TH1F h1(name.c_str(), title.c_str(), c_numChannels, 0, c_numChannels);
      h1.SetXTitle("channel number");
      h1.SetYTitle("hits per channel");
      m_hits1D.push_back(h1);

      name = "timeHits_slot" + slotNum;
      title = "hit time vs. channel for slot " + slotNum;
      TH2F h2(name.c_str(), title.c_str(), c_numChannels, 0, c_numChannels,
              200, 0.0, 20.0);
      h2.SetXTitle("channel number");
      h2.SetYTitle("time [ns]");
      m_hits2D.push_back(h2);
    }

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

  void TOPChannelT0CalibratorModule::event()
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
    const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();

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
      auto& finders = m_finders[sub][module];
      const auto& binCenters = finders[0].getBinCenters();
      for (unsigned ibin = 0; ibin < binCenters.size(); ibin++) {
        double t0 = binCenters[ibin];
        const auto& pixelLogLs = pdfConstructor.getPixelLogLs(t0, m_sigmaSmear);
        for (unsigned channel = 0; channel < c_numChannels; channel++) {
          int pix = chMapper.getPixelID(channel) - 1;
          finders[channel].add(ibin, -2 * pixelLogLs[pix].logL);
        }
      }

      // fill histograms of hits
      m_numPhotons = 0;
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getModuleID() != trk.getModuleID()) continue;
        if (digit.getTime() < timeMin) continue;
        if (digit.getTime() > timeMax) continue;
        m_numPhotons++;
        auto& h1 = m_hits1D[module];
        h1.Fill(digit.getChannel());
        auto& h2 = m_hits2D[module];
        h2.Fill(digit.getChannel(), digit.getTime());
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

    // merge two statistically independent finders and store results into histograms

    for (unsigned module = 0; module < c_numModules; module++) {
      int moduleID = module + 1;

      std::string slotNum = std::to_string(moduleID);
      if (moduleID < 10) slotNum = "0" + slotNum;

      std::string name = "channelT0_slot" + slotNum;
      std::string title = "Relative channel T0 for slot " + slotNum;
      TH1F h_channelT0(name.c_str(), title.c_str(), c_numChannels, 0, c_numChannels);
      h_channelT0.SetXTitle("channel number");
      h_channelT0.SetYTitle("relative channel T0 [ns]");

      for (unsigned channel = 0; channel < c_numChannels; channel++) {
        auto& finder = m_finders[0][module][channel].add(m_finders[1][module][channel]);
        const auto& minimum = finder.getMinimum();
        if (minimum.valid) {
          h_channelT0.SetBinContent(channel + 1, minimum.position);
          h_channelT0.SetBinError(channel + 1, minimum.error * scaleError);
        }
      }

      h_channelT0.Write();

    }

    // merge all finders of a slot to find module T0

    TH1F h_moduleT0("moduleT0", "Relative module T0",
                    c_numModules, 0.5, c_numModules + 0.5);
    h_moduleT0.SetXTitle("slot number");
    h_moduleT0.SetYTitle("relative module T0 [ns]");
    auto finderCommon = m_finders[0][0][0];
    finderCommon.clear();
    for (unsigned module = 0; module < c_numModules; module++) {
      auto finder = m_finders[0][module][0];
      finder.clear();
      for (unsigned channel = 0; channel < c_numChannels; channel++) {
        finder.add(m_finders[0][module][channel]);
      }
      finderCommon.add(finder);
      const auto& minimum = finder.getMinimum();
      if (minimum.valid) {
        h_moduleT0.SetBinContent(module + 1, minimum.position);
        h_moduleT0.SetBinError(module + 1, minimum.error * scaleError);
      }
    }
    h_moduleT0.Write();

    // find common T0

    TH1F h_commonT0("commonT0", "Relative common T0", 1, 0, 1);
    h_commonT0.SetYTitle("relative common T0 [ns]");
    const auto& minimum = finderCommon.getMinimum();
    if (minimum.valid) {
      h_commonT0.SetBinContent(1, minimum.position);
      h_commonT0.SetBinError(1, minimum.error * scaleError);
    }
    h_commonT0.Write();

    // write other histograms and ntuple; close the file

    for (auto& h : m_hits1D) h.Write();
    for (auto& h : m_hits2D) h.Write();

    m_tree->Write();
    m_file->Close();

    B2RESULT("Results available in " << m_outFileName);
  }


} // end Belle2 namespace

