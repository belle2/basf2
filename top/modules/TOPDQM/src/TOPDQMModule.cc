/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/TOPDQM/TOPDQMModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <tracking/dataobjects/ExtHit.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include "TDirectory.h"
#include <boost/format.hpp>
#include <algorithm>

using namespace std;
using boost::format;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDQM);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDQMModule::TOPDQMModule() : HistoModule()
  {
    // set module description (e.g. insert text)
    setDescription("TOP DQM histogrammer");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("TOP"));
    addParam("momentumCut", m_momentumCut,
             "momentum cut used to histogram number of photons per track", 0.5);
  }


  TOPDQMModule::~TOPDQMModule()
  {
  }

  void TOPDQMModule::defineHisto()
  {
    // Create a separate histogram directory and cd into it.

    TDirectory* oldDir = gDirectory;
    oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

    // Variables needed for booking

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_numModules = geo->getNumModules();
    double bunchTimeSep = geo->getNominalTDC().getSyncTimeBase() / 24;

    // Histograms

    m_BoolEvtMonitor = new TH1D("BoolEvtMonitor", "Event synchronization", 2, -0.5, 1.5);
    m_BoolEvtMonitor->GetYaxis()->SetTitle("number of digits");
    m_BoolEvtMonitor->GetXaxis()->SetBinLabel(1, "synchronized");
    m_BoolEvtMonitor->GetXaxis()->SetBinLabel(2, "de-synchronized");
    m_BoolEvtMonitor->GetXaxis()->SetLabelSize(0.05);
    m_BoolEvtMonitor->GetXaxis()->SetAlphanumeric();
    m_BoolEvtMonitor->SetMinimum(0);

    m_window_vs_slot = new TH2F("window_vs_slot", "Asic windows", 16, 0.5, 16.5, 512, 0, 512);
    m_window_vs_slot->SetXTitle("slot number");
    m_window_vs_slot->SetYTitle("window number w.r.t reference window");
    m_window_vs_slot->SetStats(kFALSE);
    m_window_vs_slot->SetMinimum(0);
    m_window_vs_slot->GetXaxis()->SetNdivisions(16);

    int nbinsT0 = 75;
    double rangeT0 = nbinsT0 * bunchTimeSep;
    m_eventT0 = new TH1F("eventT0", "Event T0; event T0 [ns]; events per bin", nbinsT0, -rangeT0 / 2, rangeT0 / 2);

    m_bunchOffset = new TH1F("bunchOffset", "Bunch offset", 100, -bunchTimeSep / 2, bunchTimeSep / 2);
    m_bunchOffset->SetXTitle("offset [ns]");
    m_bunchOffset->SetYTitle("events per bin");
    m_bunchOffset->SetMinimum(0);

    m_time = new TH1F("goodHitTimes", "Time distribution of good hits", 1000, -20, 80);
    m_time->SetXTitle("time [ns]");
    m_time->SetYTitle("hits per bin");

    m_timeBG = new TH1F("goodHitTimesBG", "Time distribution of good hits (background)", 1000, -20, 80);
    m_timeBG->SetXTitle("time [ns]");
    m_timeBG->SetYTitle("hits per bin");

    m_signalHits = new TProfile("signalHits", "Number of good hits per track in [0, 50] ns", 16, 0.5, 16.5, 0, 1000);
    m_signalHits->SetXTitle("slot number");
    m_signalHits->SetYTitle("hits per track");
    m_signalHits->SetMinimum(0);
    m_signalHits->GetXaxis()->SetNdivisions(16);

    m_backgroundHits = new TProfile("backgroundHits", "Number of good hits pet track in [-50, 0] ns", 16, 0.5, 16.5, 0, 1000);
    m_backgroundHits->SetXTitle("slot number");
    m_backgroundHits->SetYTitle("hits per track");
    m_backgroundHits->SetMinimum(0);
    m_backgroundHits->GetXaxis()->SetNdivisions(16);

    m_trackHits = new TH2F("trackHits", "Number of events w/ and w/o track in the slot", 16, 0.5, 16.5, 2, 0, 2);
    m_trackHits->SetXTitle("slot number");
    m_trackHits->SetYTitle("numTracks > 0");

    int nbinsHits = 1000;
    double xmaxHits = 10000;
    m_goodHitsPerEventAll = new TH1F("goodHitsPerEventAll", "Number of good hits per event", nbinsHits, 0, xmaxHits);
    m_goodHitsPerEventAll->GetXaxis()->SetTitle("hits per event");
    m_goodHitsPerEventAll->GetYaxis()->SetTitle("entries per bin");

    m_badHitsPerEventAll = new TH1F("badHitsPerEventAll", "Number of junk hits per event", nbinsHits, 0, xmaxHits);
    m_badHitsPerEventAll->GetXaxis()->SetTitle("hits per event");
    m_badHitsPerEventAll->GetYaxis()->SetTitle("entries per bin");

    int nbinsTDC = 400;
    double xminTDC = -100;
    double xmaxTDC = 700;
    m_goodTDCAll = new TH1F("goodTDCAll", "Raw time distribution of good hits", nbinsTDC, xminTDC, xmaxTDC);
    m_goodTDCAll->SetXTitle("raw time [samples]");
    m_goodTDCAll->SetYTitle("hits per sample");

    m_badTDCAll = new TH1F("badTDCAll", "Raw time distribution of junk hits", nbinsTDC, xminTDC, xmaxTDC);
    m_badTDCAll->SetXTitle("raw time [samples]");
    m_badTDCAll->SetYTitle("hits per sample");

    m_TOPOccAfterInjLER  = new TH1F("TOPOccInjLER", "TOPOccInjLER/Time;Time in #mus;Nhits/Time (#mus bins)", 4000, 0, 20000);
    m_TOPOccAfterInjHER  = new TH1F("TOPOccInjHER", "TOPOccInjHER/Time;Time in #mus;Nhits/Time (#mus bins)", 4000, 0, 20000);
    m_TOPEOccAfterInjLER  = new TH1F("TOPEOccInjLER", "TOPEOccInjLER/Time;Time in #mus;Triggers/Time (#mus bins)", 4000, 0, 20000);
    m_TOPEOccAfterInjHER  = new TH1F("TOPEOccInjHER", "TOPEOccInjHER/Time;Time in #mus;Triggers/Time (#mus bins)", 4000, 0, 20000);

    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;
      string name, title;
      TH1F* h1 = 0;
      TH2F* h2 = 0;

      name = str(format("window_vs_asic_%1%") % (module));
      title = str(format("Asic windows for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 512, 0, 512);
      h2->SetStats(kFALSE);
      h2->SetXTitle("ASIC number");
      h2->SetYTitle("window number w.r.t reference window");
      h2->SetMinimum(0);
      m_window_vs_asic.push_back(h2);

      name = str(format("good_hits_xy_%1%") % (module));
      title = str(format("Distribution of good hits for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("pixel column");
      h2->GetYaxis()->SetTitle("pixel row");
      h2->SetMinimum(0);
      m_goodHitsXY.push_back(h2);

      name = str(format("bad_hits_xy_%1%") % (module));
      title = str(format("Distribution of junk hits for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("pixel column");
      h2->GetYaxis()->SetTitle("pixel row");
      h2->SetMinimum(0);
      m_badHitsXY.push_back(h2);

      name = str(format("good_hits_asics_%1%") % (module));
      title = str(format("Distribution of good hits for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 8, 0, 8);
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("ASIC number");
      h2->GetYaxis()->SetTitle("ASIC channel");
      h2->SetMinimum(0);
      m_goodHitsAsics.push_back(h2);

      name = str(format("bad_hits_asics_%1%") % (module));
      title = str(format("Distribution of junk hits for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 8, 0, 8);
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("ASIC number");
      h2->GetYaxis()->SetTitle("ASIC channel");
      h2->SetMinimum(0);
      m_badHitsAsics.push_back(h2);

      name = str(format("good_TDC_%1%") % (module));
      title = str(format("Raw time distribution of good hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), nbinsTDC, xminTDC, xmaxTDC);
      h1->GetXaxis()->SetTitle("raw time [samples]");
      h1->GetYaxis()->SetTitle("hits per sample");
      m_goodTDC.push_back(h1);

      name = str(format("bad_TDC_%1%") % (module));
      title = str(format("Raw time distribution of junk hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), nbinsTDC, xminTDC, xmaxTDC);
      h1->GetXaxis()->SetTitle("raw time [samples]");
      h1->GetYaxis()->SetTitle("hits per sample");
      m_badTDC.push_back(h1);

      name = str(format("good_timing_%1%") % (module));
      title = str(format("Time distribution of good hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), 200, -20, 80);
      h1->GetXaxis()->SetTitle("time [ns]");
      h1->GetYaxis()->SetTitle("hits per time bin");
      m_goodTiming.push_back(h1);

      name = str(format("good_timing_%1%BG") % (module));
      title = str(format("Time distribution of good hits (background) for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), 200, -20, 80);
      h1->GetXaxis()->SetTitle("time [ns]");
      h1->GetYaxis()->SetTitle("hits per time bin");
      m_goodTimingBG.push_back(h1);

      name = str(format("good_channel_hits_%1%") % (module));
      title = str(format("Distribution of good hits for slot #%1%") % (module));
      int numPixels = geo->getModule(i + 1).getPMTArray().getNumPixels();
      h1 = new TH1F(name.c_str(), title.c_str(), numPixels, 0, numPixels);
      h1->GetXaxis()->SetTitle("channel number");
      h1->GetYaxis()->SetTitle("hits per channel");
      h1->SetMinimum(0);
      m_goodChannelHits.push_back(h1);

      name = str(format("bad_channel_hits_%1%") % (module));
      title = str(format("Distribution of junk hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), numPixels, 0, numPixels);
      h1->GetXaxis()->SetTitle("channel number");
      h1->GetYaxis()->SetTitle("hits per channel");
      h1->SetMinimum(0);
      m_badChannelHits.push_back(h1);

      name = str(format("pulseHeights_%1%") % (module));
      title = str(format("Average pulse heights for slot #%1%") % (module));
      auto* prof = new TProfile(name.c_str(), title.c_str(), 32, 0.5, 32.5, 0, 2000);
      prof->GetXaxis()->SetTitle("PMT number");
      prof->GetYaxis()->SetTitle("pulse height [ADC counts]");
      prof->SetMarkerStyle(20);
      prof->SetMinimum(0);
      m_pulseHeights.push_back(prof);
    }

    // cd back to root directory
    oldDir->cd();
  }

  void TOPDQMModule::initialize()
  {
    // Register histograms (calls back defineHisto)

    REG_HISTOGRAM;

    // register dataobjects

    m_rawFTSW.isOptional(); /// better use isRequired(), but RawFTSW is not in sim
    m_digits.isRequired();
    m_recBunch.isOptional();
    m_timeZeros.isOptional();
    m_tracks.isOptional();

  }

  void TOPDQMModule::beginRun()
  {
    m_BoolEvtMonitor->Reset();
    m_window_vs_slot->Reset();
    m_eventT0->Reset();
    m_bunchOffset->Reset();
    m_time->Reset();
    m_timeBG->Reset();
    m_signalHits->Reset();
    m_backgroundHits->Reset();
    m_trackHits->Reset();
    m_goodTDCAll->Reset();
    m_badTDCAll->Reset();
    m_goodHitsPerEventAll->Reset();
    m_badHitsPerEventAll->Reset();
    m_TOPOccAfterInjLER->Reset();
    m_TOPOccAfterInjHER->Reset();
    m_TOPEOccAfterInjLER->Reset();
    m_TOPEOccAfterInjHER->Reset();

    for (int i = 0; i < m_numModules; i++) {
      m_window_vs_asic[i]->Reset();
      m_goodHitsXY[i]->Reset();
      m_badHitsXY[i]->Reset();
      m_goodHitsAsics[i]->Reset();
      m_badHitsAsics[i]->Reset();
      m_goodTDC[i]->Reset();
      m_badTDC[i]->Reset();
      m_goodTiming[i]->Reset();
      m_goodTimingBG[i]->Reset();
      m_goodChannelHits[i]->Reset();
      m_badChannelHits[i]->Reset();
      m_pulseHeights[i]->Reset();
    }
  }

  void TOPDQMModule::event()
  {

    // check if event time is reconstructed; distinguish collision data and cosmics

    bool recBunchValid = false;
    bool cosmics = false;
    if (m_recBunch.isValid()) { // collision data
      recBunchValid = m_recBunch->isReconstructed(); // event time is reconstructed
    } else if (m_timeZeros.getEntries() ==  1) { // cosmics w/ reconstructed event time
      cosmics = true;
      m_eventT0->Fill(m_timeZeros[0]->getTime());
    }

    // fill bunch offset

    if (recBunchValid) {
      double t0 = m_commonT0->isRoughlyCalibrated() ? m_commonT0->getT0() : 0;
      m_bunchOffset->Fill(m_recBunch->getCurrentOffset() - t0);
      m_eventT0->Fill(m_recBunch->getTime());
    }

    // fill event desynchronization

    if (m_digits.getEntries() > 0) {
      for (const auto& digit : m_digits) {
        int x = digit.getFirstWindow() != m_digits[0]->getFirstWindow() ? 1 : 0 ;
        m_BoolEvtMonitor->Fill(x);
      }
    }

    // count tracks in the modules and store the momenta

    std::vector<int> numTracks(m_numModules, 0);
    std::vector<double> trackMomenta(m_numModules, 0);
    for (const auto& track : m_tracks) {
      const auto* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
      if (not fitResult) continue;
      int slot = getModuleID(track);
      if (slot == 0) continue;
      numTracks[slot - 1]++;
      trackMomenta[slot - 1] = std::max(trackMomenta[slot - 1], fitResult->getMomentum().R());
    }

    // count events w/ and w/o track in the slot

    if (recBunchValid or cosmics) {
      for (size_t i = 0; i < numTracks.size(); i++) {
        bool hit = numTracks[i] > 0;
        m_trackHits->Fill(i + 1, hit);
      }
    }

    // select modules for counting hits in signal and background time windows

    std::vector<bool> selectedSlots(m_numModules, false);
    for (size_t i = 0; i < selectedSlots.size(); i++) {
      selectedSlots[i] = (recBunchValid or cosmics) and numTracks[i] == 1 and trackMomenta[i] > m_momentumCut;
    }

    // prepare counters

    int nHits_good = 0;
    int nHits_bad = 0;
    std::vector<int> numSignalHits(m_numModules, 0);
    std::vector<int> numBackgroundHits(m_numModules, 0);

    // loop over digits, fill histograms and increment counters

    for (const auto& digit : m_digits) {
      int slot = digit.getModuleID();
      if (slot < 1 or slot > m_numModules) {
        B2ERROR("Invalid slot ID found in TOPDigits: ID = " << slot);
        continue;
      }
      int asic_no = digit.getChannel() / 8;
      int asic_ch = digit.getChannel() % 8;

      m_window_vs_slot->Fill(digit.getModuleID(), digit.getRawTime() / 64 + 220);
      m_window_vs_asic[slot - 1]->Fill(asic_no, digit.getRawTime() / 64 + 220);

      if (digit.getHitQuality() != TOPDigit::c_Junk) { // good hits
        m_goodHitsXY[slot - 1]->Fill(digit.getPixelCol(), digit.getPixelRow());
        m_goodHitsAsics[slot - 1]->Fill(asic_no, asic_ch);
        m_goodTDC[slot - 1]->Fill(digit.getRawTime());
        m_goodTDCAll->Fill(digit.getRawTime());
        m_goodChannelHits[slot - 1]->Fill(digit.getChannel());
        m_pulseHeights[slot - 1]->Fill(digit.getPMTNumber(), digit.getPulseHeight());
        nHits_good++;
        if (digit.hasStatus(TOPDigit::c_EventT0Subtracted)) {
          double time = digit.getTime();
          if (cosmics) time += 10; // move for 10 ns in order to get the complete signal at positive times
          if (numTracks[slot - 1] > 0) {
            m_goodTiming[slot - 1]->Fill(time);
            m_time->Fill(time);
          } else {
            m_goodTimingBG[slot - 1]->Fill(time);
            m_timeBG->Fill(time);
          }
          if (selectedSlots[slot - 1] and abs(time) < 50) {
            if (time > 0) numSignalHits[slot - 1]++;
            else numBackgroundHits[slot - 1]++;
          }
        }
      } else { // bad hits: FE not valid, pedestal jump, too short or too wide pulses
        m_badHitsXY[slot - 1]->Fill(digit.getPixelCol(), digit.getPixelRow());
        m_badHitsAsics[slot - 1]->Fill(asic_no, asic_ch);
        m_badTDC[slot - 1]->Fill(digit.getRawTime());
        m_badTDCAll->Fill(digit.getRawTime());
        m_badChannelHits[slot - 1]->Fill(digit.getChannel());
        nHits_bad++;
      }
    }

    // histogram counters

    m_goodHitsPerEventAll->Fill(nHits_good);
    m_badHitsPerEventAll->Fill(nHits_bad);
    for (int slot = 1; slot <= m_numModules; slot++) {
      if (selectedSlots[slot - 1]) {
        m_signalHits->Fill(slot, numSignalHits[slot - 1]);
        m_backgroundHits->Fill(slot, numBackgroundHits[slot - 1]);
      }
    }

    // fill injection histograms

    for (auto& it : m_rawFTSW) {
      B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
              (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
              it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));
      auto difference = it.GetTimeSinceLastInjection(0);
      if (difference != 0x7FFFFFFF) {
        unsigned int nentries = m_digits.getEntries();
        float diff2 = difference / 127.; //  127MHz clock ticks to us, inexact rounding
        if (it.GetIsHER(0)) {
          m_TOPOccAfterInjHER->Fill(diff2, nentries);
          m_TOPEOccAfterInjHER->Fill(diff2);
        } else {
          m_TOPOccAfterInjLER->Fill(diff2, nentries);
          m_TOPEOccAfterInjLER->Fill(diff2);
        }
      }
    }

  }


  int TOPDQMModule::getModuleID(const Track& track) const
  {
    Const::EDetector myDetID = Const::EDetector::TOP;
    int pdgCode = std::abs(Const::pion.getPDGCode());

    RelationVector<ExtHit> extHits = track.getRelationsWith<ExtHit>();
    for (const auto& extHit : extHits) {
      if (std::abs(extHit.getPdgCode()) != pdgCode) continue;
      if (extHit.getDetectorID() != myDetID) continue;
      if (extHit.getCopyID() < 1 or extHit.getCopyID() > m_numModules) continue;
      return extHit.getCopyID();
    }

    return 0;
  }


} // end Belle2 namespace

