/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Dan Santel, Boqun Wang                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager
#include <framework/core/HistoModule.h>

// Own include
#include <top/modules/TOPDQM/TOPDQMModule.h>
#include <top/geometry/TOPGeometryPar.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
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
    int numTDCbins = geo->getNominalTDC().getNumWindows() * 64;

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
    m_goodHits = new TH1F("good_hits", "Number of good hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_badHits = new TH1F("bad_hits", "Number of bad hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_goodHits->SetOption("LIVE");
    m_badHits->SetOption("LIVE");

    m_goodHits->GetXaxis()->SetTitle("slot no.");
    m_goodHits->GetYaxis()->SetTitle("hits / event");
    m_badHits->GetXaxis()->SetTitle("slot no.");
    m_badHits->GetYaxis()->SetTitle("hits / event");

    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;
      string name, title;
      TH1F* h1;
      TH2F* h2;

      name = str(format("good_hits_xy_%1%") % (module));
      title = str(format("Number of good hits in x-y for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("column");
      h2->GetYaxis()->SetTitle("row");
      m_goodHitsXY.push_back(h2);

      name = str(format("bad_hits_xy_%1%") % (module));
      title = str(format("Number of bad hits in x-y for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("column");
      h2->GetYaxis()->SetTitle("row");
      m_badHitsXY.push_back(h2);

      name = str(format("good_hits_asics_%1%") % (module));
      title = str(format("Number of good hits for asics for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 8, 0, 8);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("column");
      h2->GetYaxis()->SetTitle("row");
      m_goodHitsAsics.push_back(h2);

      name = str(format("bad_hits_asics_%1%") % (module));
      title = str(format("Number of bad hits for asics for slot #%1%") % (module));
      h2 = new TH2F(name.c_str(), title.c_str(), 64, 0, 64, 8, 0, 8);
      h2->SetOption("LIVE");
      h2->SetStats(kFALSE);
      h2->GetXaxis()->SetTitle("column");
      h2->GetYaxis()->SetTitle("row");
      m_badHitsAsics.push_back(h2);

      name = str(format("good_TDC_%1%") % (module));
      title = str(format("TDC distribution of good hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), numTDCbins, 0, numTDCbins);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("TDC");
      m_goodTdc.push_back(h1);

      name = str(format("bad_TDC_%1%") % (module));
      title = str(format("TDC distribution of bad hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), numTDCbins, 0, numTDCbins);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("TDC");
      m_badTdc.push_back(h1);

      name = str(format("good_timing_%1%") % (module));
      title = str(format("Timing distribution of good hits for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), 100, -20, 80);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("time [ns]");
      m_goodTiming.push_back(h1);

      name = str(format("good_channel_hits_%1%") % (module));
      title = str(format("Number of good hits by channel for slot #%1%") % (module));
      int numPixels = geo->getModule(i + 1).getPMTArray().getNumPixels();
      h1 = new TH1F(name.c_str(), title.c_str(), numPixels, 0, numPixels);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("pixel");
      m_goodChannelHits.push_back(h1);

      name = str(format("bad_channel_hits_%1%") % (module));
      title = str(format("Number of bad hits by channel for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), numPixels, 0, numPixels);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("pixel");
      m_badChannelHits.push_back(h1);

      name = str(format("good_hits_per_event%1%") % (module));
      title = str(format("Number of good hits per event for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("hits / event");
      m_goodHitsPerEvent.push_back(h1);

      name = str(format("bad_hits_per_event%1%") % (module));
      title = str(format("Number of bad hits per event for slot #%1%") % (module));
      h1 = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
      h1->SetOption("LIVE");
      h1->GetXaxis()->SetTitle("hits / event");
      m_badHitsPerEvent.push_back(h1);
    }

    // cd back to root directory
    oldDir->cd();
  }

  void TOPDQMModule::initialize()
  {
    // Register histograms (calls back defineHisto)
    REG_HISTOGRAM;

    // register dataobjects
    m_digits.isRequired();
    m_tracks.isOptional();

  }

  void TOPDQMModule::beginRun()
  {
    m_recoTimeDiff->Reset();
    m_recoTimeDiff_Phic->Reset();
    m_recoPull->Reset();
    m_recoPull_Phic->Reset();
    m_recoTime->Reset();
    m_recoTimeBg->Reset();
    m_recoTimeMinT0->Reset();

    m_goodHits->Reset();
    m_badHits->Reset();
    for (int i = 0; i < m_numModules; i++) {
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
    }
  }

  void TOPDQMModule::event()
  {

    std::vector<int> n_good(16, 0);
    std::vector<int> n_bad(16, 0);
    for (const auto& digit : m_digits) {
      int i = digit.getModuleID() - 1;
      if (i < 0 || i >= m_numModules) {
        B2ERROR("Invalid module ID found in TOPDigits: ID = " << i + 1);
        continue;
      }

      int ch = digit.getChannel();
      int asic_no = ch / 8, asic_ch = ch % 8;

      if (digit.getHitQuality() == TOPDigit::c_Good) { // good hits
        m_goodHits->Fill(i + 1);
        m_goodHitsXY[i]->Fill(digit.getPixelCol(), digit.getPixelRow());
        m_goodHitsAsics[i]->Fill(asic_no, asic_ch);
        m_goodTdc[i]->Fill(digit.getRawTime());
        m_goodTiming[i]->Fill(digit.getTime());
        m_goodChannelHits[i]->Fill(digit.getChannel());
        n_good[i]++;
      } else { // other hits = background hits
        m_badHits->Fill(i + 1);
        m_badHitsXY[i]->Fill(digit.getPixelCol(), digit.getPixelRow());
        m_badHitsAsics[i]->Fill(asic_no, asic_ch);
        m_badTdc[i]->Fill(digit.getRawTime());
        m_badChannelHits[i]->Fill(digit.getChannel());
        n_bad[i]++;
      }
    }

    for (int i = 0; i < 16; i++) {
      if (n_good[i] != 0)
        m_goodHitsPerEvent[i]->Fill(n_good[i]);
      if (n_bad[i] != 0)
        m_badHitsPerEvent[i]->Fill(n_bad[i]);
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

  }


  void TOPDQMModule::endRun()
  {
  }

  void TOPDQMModule::terminate()
  {
  }


} // end Belle2 namespace

