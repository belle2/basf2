/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Module manager
#include <framework/core/HistoModule.h>

// Own include
#include <top/modules/TOPDQM/TOPDQMModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobject classes
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPPull.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <mdst/dataobjects/Track.h>

// root
#include "TVector3.h"
#include "TDirectory.h"

// boost
#include <boost/format.hpp>

using namespace std;
using boost::format;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDQM)

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
             "momentum cut used to histogram pulls etc.", 2.0);
    addParam("pValueCut", m_pValueCut,
             "track p-value cut used to histogram pulls etc.", 0.001);
    addParam("usePionID", m_usePionID,
             "use pion ID from TOP to histogram pulls etc.", true);
    addParam("cutNphot", m_cutNphot, "Cut on total number of photons", 3);
  }


  TOPDQMModule::~TOPDQMModule()
  {
  }

  void TOPDQMModule::defineHisto()
  {
    // Create a separate histogram directory and cd into it.
    TDirectory* oldDir = gDirectory;
    oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

    // variables needed for booking
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_numModules = geo->getNumModules();
    double bunchTimeSep = geo->getNominalTDC().getSyncTimeBase() / 24;

    m_BoolEvtMonitor = new TH1F("BoolEvtMonitor", "Event desynchronization monitoring",
                                2, -0.5, 1.5);
    m_BoolEvtMonitor->GetXaxis()->SetTitle("good/bad event entries");

    m_recoTime = new TH1F("recoTime", "reco: time distribution",
                          500, 0, 50);
    m_recoTime->GetXaxis()->SetTitle("time [ns]");

    m_recoTimeBg = new TH1F("recoTimeBg", "reco: time distribution (bkg)",
                            500, 0, 50);
    m_recoTimeBg->GetXaxis()->SetTitle("time [ns]");

    m_recoTimeMinT0 = new TH1F("recoTimeMinT0", "reco: time in respect to first PDF peak",
                               200, -10, 10);
    m_recoTimeMinT0->GetXaxis()->SetTitle("time [ns]");

    m_recoTimeDiff = new TH1F("recoTimeDiff", "reco: time resolution",
                              100, -1.0, 1.0);
    m_recoTimeDiff->GetXaxis()->SetTitle("time residual [ns]");

    m_recoPull = new TH1F("recoPull", "reco: pulls",
                          100, -10, 10);
    m_recoPull->GetXaxis()->SetTitle("pull");

    m_recoTimeDiff_Phic = new TH2F("recoTimeDiff_Phic",
                                   "reco: time resolution vs. phiCer",
                                   90, -180, 180, 100, -1.0, 1.0);
    m_recoTimeDiff_Phic->GetXaxis()->SetTitle("Cerenkov azimuthal angle [deg]");
    m_recoTimeDiff_Phic->GetYaxis()->SetTitle("time residuals [ns]");

    m_recoPull_Phic = new TProfile("recoPull_Phic",
                                   "reco: pulls vs phiCer",
                                   90, -180, 180, -10, 10, "S");
    m_recoPull_Phic->GetXaxis()->SetTitle("Cerenkov azimuthal angle [deg]");
    m_recoPull_Phic->GetYaxis()->SetTitle("pulls");

    // Histograms from TOPDataQualtiyOnline
    m_goodHits = new TH1F("goodHits", "Number of good hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_badHits = new TH1F("badHits", "Number of bad hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_goodHits->SetOption("LIVE");
    m_badHits->SetOption("LIVE");
    m_goodHits->SetMinimum(0);
    m_badHits->SetMinimum(0);

    m_goodHits->GetXaxis()->SetTitle("slot no.");
    m_goodHits->GetYaxis()->SetTitle("hits / slot");
    m_badHits->GetXaxis()->SetTitle("slot number");
    m_badHits->GetYaxis()->SetTitle("hits / slot");

    // New histograms
    m_window_vs_slot = new TH2F("window_vs_slot", "Distribution of hits: raw timing",
                                16, 0.5, 16.5, 512, 0, 512);
    m_window_vs_slot->SetXTitle("slot number");
    m_window_vs_slot->SetYTitle("window number w.r.t reference window");
    m_window_vs_slot->SetOption("LIVE");
    m_window_vs_slot->SetStats(kFALSE);

    m_bunchOffset = new TH1F("bunchOffset", "Reconstructed bunch: current offset",
                             100, -bunchTimeSep / 2, bunchTimeSep / 2);
    m_bunchOffset->SetXTitle("offset [ns]");
    m_bunchOffset->SetYTitle("events/bin");
    m_bunchOffset->SetOption("LIVE");
    m_bunchOffset->SetMinimum(0);

    m_time = new TH1F("goodHitTimes", "Time distribution of good hits",
                      1000, -20, 80);
    m_time->SetXTitle("time [ns]");
    m_time->SetYTitle("hits/bin");
    m_time->SetOption("LIVE");
    m_time->SetMinimum(0);

    int MaxEvents(1000);
    m_goodHitsPerEventAll = new TH1F("goodHitsPerEventAll", "Number of good hits per event", MaxEvents, 0, MaxEvents);
    m_badHitsPerEventAll = new TH1F("badHitsPerEventAll", "Number of bad hits per event", MaxEvents, 0, MaxEvents);
    m_goodHitsPerEventAll->SetOption("LIVE");
    m_badHitsPerEventAll->SetOption("LIVE");
    m_goodHitsPerEventAll->SetMinimum(0);
    m_badHitsPerEventAll->SetMinimum(0);
    m_goodHitsPerEventAll->GetXaxis()->SetTitle("hits / event");
    m_goodHitsPerEventAll->GetYaxis()->SetTitle("Events");
    m_badHitsPerEventAll->GetXaxis()->SetTitle("hits / event");
    m_badHitsPerEventAll->GetYaxis()->SetTitle("Events");

    int MaxRawTime(800);
    int BinNumRT(400);
    m_goodTDCAll = new TH1F("goodTDCAll", "Raw time distribution of good hits",
                            BinNumRT, 0, MaxRawTime);
    m_goodTDCAll->SetXTitle("raw time [samples]");
    m_goodTDCAll->SetYTitle("hits / sample");
    m_goodTDCAll->SetOption("LIVE");
    m_goodTDCAll->SetMinimum(0);

    m_badTDCAll = new TH1F("badTDCAll", "Raw time distribution of bad hits",
                           BinNumRT, 0, MaxRawTime);
    m_badTDCAll->SetXTitle("raw time [samples]");
    m_badTDCAll->SetYTitle("hits / sample");
    m_badTDCAll->SetOption("LIVE");
    m_badTDCAll->SetMinimum(0);

    m_goodHitsPerEventProf = new TProfile("goodHitsPerEventProf", "Good hits per event vs. slot number",
                                          16, 0.5, 16.5, 0, MaxEvents);
    m_goodHitsPerEventProf->SetXTitle("slot number");
    m_goodHitsPerEventProf->SetYTitle("hits per event");
    m_goodHitsPerEventProf->SetOption("LIVE");
    m_goodHitsPerEventProf->SetStats(kFALSE);
    m_goodHitsPerEventProf->SetMinimum(0);

    m_badHitsPerEventProf = new TProfile("badHitsPerEventProf", "Bad hits per event vs. slot number",
                                         16, 0.5, 16.5, 0, MaxEvents);
    m_badHitsPerEventProf->SetXTitle("slot number");
    m_badHitsPerEventProf->SetYTitle("hits per event");
    m_badHitsPerEventProf->SetOption("LIVE");
    m_badHitsPerEventProf->SetStats(kFALSE);
    m_badHitsPerEventProf->SetMinimum(0);

    m_TOPOccAfterInjLER  = new TH1F("TOPOccInjLER", "TOPOccInjLER/Time;Time in #mus;Nhits/Time (#mus bins)", 4000, 0, 20000);
    m_TOPOccAfterInjHER  = new TH1F("TOPOccInjHER", "TOPOccInjHER/Time;Time in #mus;Nhits/Time (#mus bins)", 4000, 0, 20000);
    m_TOPEOccAfterInjLER  = new TH1F("TOPEOccInjLER", "TOPEOccInjLER/Time;Time in #mus;Triggers/Time (#mus bins)", 4000, 0, 20000);
    m_TOPEOccAfterInjHER  = new TH1F("TOPEOccInjHER", "TOPEOccInjHER/Time;Time in #mus;Triggers/Time (#mus bins)", 4000, 0, 20000);

    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;
      string name, title;
      TH1F* h1 = 0;
      TH2F* h2 = 0;
      TProfile2D* h3 = 0;

      name = str(format("window_vs_asic_%1%") % (module));
      title = str(format("Distribution of hits: raw timing for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 512, 0, 512);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->SetXTitle("ASIC number");
      h2->SetYTitle("window number w.r.t reference window");
      h2->SetMinimum(0);
      m_window_vs_asic.push_back(h2);

      name = str(format("good_hits_xy_%1%") % (module));
      title = str(format("Number of good hits in x-y for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("pixel column");
      h2->GetYaxis()->SetTitle("pixel row");
      h2->SetMinimum(0);
      m_goodHitsXY.push_back(h2);

      name = str(format("bad_hits_xy_%1%") % (module));
      title = str(format("Number of bad hits in x-y for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("pixel column");
      h2->GetYaxis()->SetTitle("pixel row");
      h2->SetMinimum(0);
      m_badHitsXY.push_back(h2);

      name = str(format("good_hits_asics_%1%") % (module));
      title = str(format("Number of good hits for asics for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 8, 0, 8);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("ASIC number");
      h2->GetYaxis()->SetTitle("ASIC channel");
      h2->SetMinimum(0);
      m_goodHitsAsics.push_back(h2);

      name = str(format("bad_hits_asics_%1%") % (module));
      title = str(format("Number of bad hits for asics for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 8, 0, 8);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("ASIC number");
      h2->GetYaxis()->SetTitle("ASIC channel");
      h2->SetMinimum(0);
      m_badHitsAsics.push_back(h2);

      name = str(format("good_TDC_%1%") % (module));
      title = str(format("Raw time distribution of good hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), BinNumRT, 0, MaxRawTime);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("raw time [samples]");
      h1->GetYaxis()->SetTitle("hits per sample");
      h1->SetMinimum(0);
      m_goodTdc.push_back(h1);

      name = str(format("bad_TDC_%1%") % (module));
      title = str(format("Raw time distribution of bad hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), BinNumRT, 0, MaxRawTime);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("raw time [samples]");
      h1->GetYaxis()->SetTitle("hits per sample");
      h1->SetMinimum(0);
      m_badTdc.push_back(h1);

      name = str(format("good_timing_%1%") % (module));
      title = str(format("Timing distribution of good hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), 100, -20, 80);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("time [ns]");
      h1->GetYaxis()->SetTitle("hits per time bin");
      h1->SetMinimum(0);
      m_goodTiming.push_back(h1);

      name = str(format("good_channel_hits_%1%") % (module));
      title = str(format("Number of good hits by channel for slot #%1%") % (module));
      int numPixels = geo->getModule(i + 1).getPMTArray().getNumPixels();
      h1 = new TH1F(name.c_str(), title.c_str(), numPixels, 0, numPixels);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("channel number");
      h1->GetYaxis()->SetTitle("hits pre channel");
      h1->SetMinimum(0);
      m_goodChannelHits.push_back(h1);

      name = str(format("bad_channel_hits_%1%") % (module));
      title = str(format("Number of bad hits by channel for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), numPixels, 0, numPixels);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("channel number");
      h1->GetYaxis()->SetTitle("hits pre channel");
      h1->SetMinimum(0);
      m_badChannelHits.push_back(h1);

      name = str(format("good_hits_per_event%1%") % (module));
      title = str(format("Number of good hits per event for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), MaxEvents, 0, MaxEvents);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("hits / event");
      h1->SetMinimum(0);
      m_goodHitsPerEvent.push_back(h1);

      name = str(format("bad_hits_per_event%1%") % (module));
      title = str(format("Number of bad hits per event for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), MaxEvents, 0, MaxEvents);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("hits / event");
      h1->SetMinimum(0);
      m_badHitsPerEvent.push_back(h1);

      name = str(format("good_hits_xy_track_%1%") % (module));
      title = str(format("Hits per track, each channel, slot #%1%") % (module));
      h3 = new TProfile2D(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5, 0, 1000);
      h3->SetOption("LIVE");
      h3->SetStats(kFALSE);
      h3->GetXaxis()->SetTitle("pixel column");
      h3->GetYaxis()->SetTitle("pixel row");
      h3->SetMinimum(0);
      m_goodHitsXYTrack.push_back(h3);

      name = str(format("good_hits_xy_track_bkg_%1%") % (module));
      title = str(format("Hits per bkg track, each channel, slot #%1%") % (module));
      h3 = new TProfile2D(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5, 0, 1000);
      h3->SetOption("LIVE");
      h3->SetStats(kFALSE);
      h3->GetXaxis()->SetTitle("pixel column");
      h3->GetYaxis()->SetTitle("pixel row");
      h3->SetMinimum(0);
      m_goodHitsXYTrackBkg.push_back(h3);
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
    m_tracks.isOptional();

  }

  void TOPDQMModule::beginRun()
  {
    m_BoolEvtMonitor->Reset();

    m_recoTimeDiff->Reset();
    m_recoTimeDiff_Phic->Reset();
    m_recoPull->Reset();
    m_recoPull_Phic->Reset();
    m_recoTime->Reset();
    m_recoTimeBg->Reset();
    m_recoTimeMinT0->Reset();

    m_goodHits->Reset();
    m_badHits->Reset();
    m_window_vs_slot->Reset();
    m_bunchOffset->Reset();
    m_time->Reset();
    m_goodTDCAll->Reset();
    m_badTDCAll->Reset();
    m_goodHitsPerEventProf->Reset();
    m_goodHitsPerEventAll->Reset();
    m_badHitsPerEventProf->Reset();
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
      m_goodTdc[i]->Reset();
      m_badTdc[i]->Reset();
      m_goodTiming[i]->Reset();
      m_goodChannelHits[i]->Reset();
      m_badChannelHits[i]->Reset();
      m_goodHitsPerEvent[i]->Reset();
      m_badHitsPerEvent[i]->Reset();
      m_goodHitsXYTrack[i]->Reset();
      m_goodHitsXYTrackBkg[i]->Reset();
    }
  }

  void TOPDQMModule::event()
  {

    bool recBunchValid = false;
    if (m_recBunch.isValid()) {
      recBunchValid = m_recBunch->isReconstructed();
    }

    if (recBunchValid) {
      m_bunchOffset->Fill(m_recBunch->getCurrentOffset());
    }

    std::vector<int> n_good(16, 0);
    std::vector<int> n_bad(16, 0);
    std::vector<int> n_good_first(16, 0);
    std::vector<int> n_good_second(16, 0);
    std::vector<int> n_good_pixel_hits(16 * 512, 0);

    int Ndigits = m_digits.getEntries();
    if (Ndigits > 0) {
      for (const auto& digit : m_digits) {
        int x = digit.getFirstWindow() != m_digits[0]->getFirstWindow() ? 1 : 0 ;
        m_BoolEvtMonitor->Fill(x);
      }
    }

    for (const auto& digit : m_digits) {
      int i = digit.getModuleID() - 1;
      if (i < 0 || i >= m_numModules) {
        B2ERROR("Invalid module ID found in TOPDigits: ID = " << i + 1);
        continue;
      }

      int ch = digit.getChannel();
      int asic_no = ch / 8, asic_ch = ch % 8;

      m_window_vs_slot->Fill(digit.getModuleID(), digit.getRawTime() / 64 + 220);
      m_window_vs_asic[i]->Fill(digit.getChannel() / 8, digit.getRawTime() / 64 + 220);

      if (digit.getHitQuality() != TOPDigit::c_Junk) { // good hits
        m_goodHits->Fill(i + 1);
        m_goodHitsXY[i]->Fill(digit.getPixelCol(), digit.getPixelRow());
        m_goodHitsAsics[i]->Fill(asic_no, asic_ch);
        m_goodTdc[i]->Fill(digit.getRawTime());
        m_goodTDCAll->Fill(digit.getRawTime());
        if (recBunchValid) {
          m_goodTiming[i]->Fill(digit.getTime());
          m_time->Fill(digit.getTime());
        }
        m_goodChannelHits[i]->Fill(digit.getChannel());
        if (digit.getTime() > 0 && digit.getTime() < 20) n_good_first[i]++;
        if (digit.getTime() > 20 && digit.getTime() < 50) n_good_second[i]++;
        n_good_pixel_hits[(digit.getModuleID() - 1) * 512 + (digit.getPixelID() - 1)]++;
        n_good[i]++;
      } else { // bad hits: FE not valid, pedestal jump, too short or too wide pulses
        m_badHits->Fill(i + 1);
        m_badHitsXY[i]->Fill(digit.getPixelCol(), digit.getPixelRow());
        m_badHitsAsics[i]->Fill(asic_no, asic_ch);
        m_badTdc[i]->Fill(digit.getRawTime());
        m_badTDCAll->Fill(digit.getRawTime());
        m_badChannelHits[i]->Fill(digit.getChannel());
        n_bad[i]++;
      }
    }

    for (int i = 0; i < 16; i++) {
      m_goodHitsPerEventProf->Fill(i + 1, n_good[i]);
      m_badHitsPerEventProf->Fill(i + 1, n_bad[i]);
      m_goodHitsPerEvent[i]->Fill(n_good[i]);
      m_goodHitsPerEventAll->Fill(n_good[i]);
      m_badHitsPerEvent[i]->Fill(n_bad[i]);
      m_badHitsPerEventAll->Fill(n_bad[i]);

      bool slot_has_track = (n_good_first[i] + n_good_second[i]) > m_cutNphot;
      for (int j = 0; j < 512; j++) {
        int col = j % 64 + 1, row = j / 64 + 1;
        if (slot_has_track)
          m_goodHitsXYTrack[i]->Fill(col, row, n_good_pixel_hits[i * 512 + j]);
        else
          m_goodHitsXYTrackBkg[i]->Fill(col, row, n_good_pixel_hits[i * 512 + j]);
      }
    }

    for (const auto& track : m_tracks) {
      const auto* trackFit = track.getTrackFitResultWithClosestMass(Const::pion);
      if (!trackFit) continue;
      if (trackFit->getMomentum().Mag() < m_momentumCut) continue;
      if (trackFit->getPValue() < m_pValueCut) continue;
      if (m_usePionID) {
        const auto* top = track.getRelated<TOPLikelihood>();
        if (!top) continue;
        if (top->getLogL_pi() < top->getLogL_K()) continue;
        if (top->getLogL_pi() < top->getLogL_p()) continue;
      }

      const auto pulls = track.getRelationsWith<TOPPull>();
      for (const auto& pull : pulls) {
        if (pull.isSignal()) {
          double phiCer = pull.getPhiCer() / Unit::deg;
          m_recoTimeDiff->Fill(pull.getTimeDiff(), pull.getWeight());
          m_recoTimeDiff_Phic->Fill(phiCer, pull.getTimeDiff(), pull.getWeight());
          m_recoPull->Fill(pull.getPull(), pull.getWeight());
          m_recoPull_Phic->Fill(phiCer, pull.getPull(), pull.getWeight());
        } else {
          m_recoTime->Fill(pull.getTime());
          m_recoTimeBg->Fill(pull.getTime(), pull.getWeight());
          m_recoTimeMinT0->Fill(pull.getTimeDiff());
        }
      }
    }

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


  void TOPDQMModule::endRun()
  {
  }

  void TOPDQMModule::terminate()
  {
  }


} // end Belle2 namespace

