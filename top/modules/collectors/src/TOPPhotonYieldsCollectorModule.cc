/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPPhotonYieldsCollectorModule.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/geometry/TOPGeometryPar.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// root
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPPhotonYieldsCollector);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPhotonYieldsCollectorModule::TOPPhotonYieldsCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for photon pixel yields aimed for PMT ageing studies and for finding optically decoupled PMT's");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("sample", m_sample, "sample type: one of dimuon or bhabha", std::string("dimuon"));
    addParam("deltaEcms", m_deltaEcms, "c.m.s energy window (half size) if sample is dimuon or bhabha", 0.1);
    addParam("dr", m_dr, "cut on POCA in r", 2.0);
    addParam("dz", m_dz, "cut on POCA in abs(z)", 4.0);
    addParam("minThresholdEffi", m_minThresholdEffi, "threshold efficiency cut to suppress unreliable calibrations", 0.7);

  }


  void TOPPhotonYieldsCollectorModule::prepare()
  {
    // input collections
    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isRequired();
    m_asicMask.isRequired();
    m_associatedPDFs.isRequired();

    // set track selector
    if (m_sample == "dimuon" or m_sample == "bhabha") {
      m_selector = TrackSelector(m_sample);
      m_selector.setDeltaEcms(m_deltaEcms);
      m_selector.setCutOnPOCA(m_dr, m_dz);
      m_selector.setCutOnLocalZ(m_minZ, m_maxZ);
    } else {
      B2ERROR("Invalid sample type '" << m_sample << "'");
    }

    // create and register histograms

    const int numModules = 16;
    const int numPixels = 512;

    // time stamp (average unix time and its standard deviation)
    auto* timeStamp = new TProfile("timeStamp", "Time stamp; ; unix time", 1, 0, 1, 0, 1.0e10, "S");
    registerObject<TProfile>("timeStamp", timeStamp);

    // number of selected tracks per slot
    auto* numTracks = new TH1F("numTracks", "Number of tracks per slot; slot number; track count", numModules, 0.5, numModules + 0.5);
    registerObject<TH1F>("numTracks", numTracks);

    // number of pixel hits in a signal time window
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "signalHits_0" + to_string(slot) : "signalHits_" + to_string(slot);
      string title = "Hits in signal window for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), numPixels, 0.5, numPixels + 0.5);
      h->SetXTitle("pixel number");
      h->SetYTitle("hit count");
      registerObject<TH1F>(name, h);
      m_signalNames.push_back(name);
    }

    // number of pixel hits in a background time window
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "bkgHits_0" + to_string(slot) : "bkgHits_" + to_string(slot);
      string title = "Hits in background window for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), numPixels, 0.5, numPixels + 0.5);
      h->SetXTitle("pixel number");
      h->SetYTitle("hit count");
      registerObject<TH1F>(name, h);
      m_bkgNames.push_back(name);
    }

    // active pixels
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "activePixels_0" + to_string(slot) : "activePixels_" + to_string(slot);
      string title = "Active pixels for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), numPixels, 0.5, numPixels + 0.5);
      h->SetXTitle("pixel number");
      h->SetYTitle("track count");
      registerObject<TH1F>(name, h);
      m_activeNames.push_back(name);
    }

    // number of pixel hits with low impact angle on photo cathode
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "alphaLow_0" + to_string(slot) : "alphaLow_" + to_string(slot);
      string title = "Hits w/ low alpha for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), numPixels, 0.5, numPixels + 0.5);
      h->SetXTitle("pixel number");
      h->SetYTitle("hit count");
      registerObject<TH1F>(name, h);
      m_alphaLowNames.push_back(name);
    }

    // number of pixel hits with high impact angle on photo cathode
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "alphaHigh_0" + to_string(slot) : "alphaHigh_" + to_string(slot);
      string title = "Hits w/ high alpha for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), numPixels, 0.5, numPixels + 0.5);
      h->SetXTitle("pixel number");
      h->SetYTitle("hit count");
      registerObject<TH1F>(name, h);
      m_alphaHighNames.push_back(name);
    }

    // pixel pulse-height distributions
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "pulseHeights_0" + to_string(slot) : "pulseHeights_" + to_string(slot);
      string title = "Pulse height distributions for slot " + to_string(slot);
      auto h = new TH2F(name.c_str(), title.c_str(), numPixels, 0.5, numPixels + 0.5, 200, 0, 2000);
      h->SetXTitle("pixel number");
      h->SetYTitle("pulse height");
      registerObject<TH2F>(name, h);
      m_pulseHeightNames.push_back(name);
    }

    // local z-distribution of tracks
    for (int slot = 1; slot <= numModules; slot++) {
      string name = (slot < 10) ? "muonZ_0" + to_string(slot) : "muonZ_" + to_string(slot);
      string title = "Track z-distribution for slot " + to_string(slot);
      auto h = new TH1F(name.c_str(), title.c_str(), 100, m_minZ, m_maxZ);
      h->SetXTitle("local z [cm]");
      h->SetYTitle("track count");
      registerObject<TH1F>(name, h);
      m_muonZNames.push_back(name);
    }

  }


  void TOPPhotonYieldsCollectorModule::collect()
  {
    // bunch must be reconstructed

    if (not m_recBunch->isReconstructed()) return;

    // loop over reconstructed tracks, make a selection and fill histograms

    for (const auto& track : m_tracks) {
      // track selection
      TOPTrack trk(track);
      if (not trk.isValid()) continue;
      if (not m_selector.isSelected(trk)) continue;

      // fill histograms
      auto timeStamp = getObjectPtr<TProfile>("timeStamp");
      timeStamp->Fill(0.5, m_eventMetaData->getTime() / 1000000000);

      int slot = trk.getModuleID();
      auto numTracks = getObjectPtr<TH1F>("numTracks");
      numTracks->Fill(slot);

      auto muonZ = getObjectPtr<TH1F>(m_muonZNames[slot - 1]);
      muonZ->Fill(m_selector.getLocalPosition().Z());

      auto signalHits = getObjectPtr<TH1F>(m_signalNames[slot - 1]);
      auto bkgHits = getObjectPtr<TH1F>(m_bkgNames[slot - 1]);
      auto pulseHeight = getObjectPtr<TH2F>(m_pulseHeightNames[slot - 1]);
      for (const auto& digit : m_digits) {
        if (digit.getModuleID() != slot) continue;
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;  // junk hit or pixel masked-out
        if (not m_thresholdEff->isCalibrated(slot, digit.getChannel())) continue;  // threshold effi. not calibrated
        double effi = m_thresholdEff->getThrEff(slot, digit.getChannel());
        if (effi < m_minThresholdEffi) continue; // to suppress possibly unreliable calibration
        if (std::abs(digit.getTime()) > m_timeWindow) continue;
        // fill signal and background hits with weight=1/effi to correct for threshold efficiency
        if (digit.getTime() > 0) {
          signalHits->Fill(digit.getPixelID(), 1 / effi);
          pulseHeight->Fill(digit.getPixelID(), digit.getPulseHeight());
        } else {
          bkgHits->Fill(digit.getPixelID(), 1 / effi);
        }
      }

      auto activePixels = getObjectPtr<TH1F>(m_activeNames[slot - 1]);
      const auto& chMapper = TOPGeometryPar::Instance()->getChannelMapper();
      for (int pixel = 1; pixel <= activePixels->GetNbinsX(); pixel++) {
        unsigned channel = chMapper.getChannel(pixel);
        if (not m_thresholdEff->isCalibrated(slot, channel)) continue; // pixel excluded in counting hits
        if (m_thresholdEff->getThrEff(slot, channel) < m_minThresholdEffi) continue; // pixel excluded in counting hits
        if (m_channelMask->isActive(slot, channel) and m_asicMask->isActive(slot, channel)) activePixels->Fill(pixel);
      }

      if (std::abs(m_selector.getLocalPosition().Z()) > m_excludedZ) {
        auto alphaLow = getObjectPtr<TH1F>(m_alphaLowNames[slot - 1]);
        auto alphaHigh = getObjectPtr<TH1F>(m_alphaHighNames[slot - 1]);
        for (const auto& digit : m_digits) {
          if (digit.getModuleID() != slot) continue;
          if (digit.getHitQuality() != TOPDigit::c_Good) continue;  // junk hit or pixel masked-out
          const auto* pdf = digit.getRelated<TOPAssociatedPDF>();
          if (not pdf) continue;
          const auto* peak = pdf->getSinglePeak();
          if (not peak) continue;  // hit associated with background
          double alpha = acos(std::abs(peak->kzd)) / Unit::deg;
          if (alpha > 60) continue;
          if (alpha < 30) alphaLow->Fill(digit.getPixelID());
          else alphaHigh->Fill(digit.getPixelID());
        }
      }
    }

  }


}
