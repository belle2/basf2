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
    addParam("ADCCutLow", m_ADCCutLow, "lower bound of ADC cut", 100);
    addParam("ADCCutHigh", m_ADCCutHigh, "higher bound of ADC cut", 2048);
    addParam("PulseWidthCutLow",  m_PulseWidthCutLow, "lower bound of PulseWidth cut", 3);
    addParam("PulseWidthCutHigh", m_PulseWidthCutHigh, "higher bound of PulseWidth cut", 10);
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
    m_particleHits = new TH1F("particle_hits", "Number of particle hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_otherHits = new TH1F("other_hits", "Number of other hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_particleHits->SetOption("LIVE");
    m_otherHits->SetOption("LIVE");

    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;

      string name1 = str(format("all_hits_xy_%1%") % (module));
      string title1 = str(format("Number of hits in x-y for module #%1%") % (module));
      TH2F* h1 = new TH2F(name1.c_str(), title1.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h1->SetOption("LIVE");
      m_allHitsXY.push_back(h1);

      string name2 = str(format("all_TDC_%1%") % (module));
      string title2 = str(format("TDC distribution for module #%1%") % (module));
      TH1F* h2 = new TH1F(name2.c_str(), title2.c_str(), numTDCbins, 0, numTDCbins);
      h2->SetOption("LIVE");
      m_allTdc.push_back(h2);

      string name3 = str(format("particle_channel_hits_%1%") % (module));
      string title3 = str(format("Number of particle hits by channel of module #%1%") % (module));
      int numPixels = geo->getModule(i + 1).getPMTArray().getNumPixels();
      TH1F* h3 = new TH1F(name3.c_str(), title3.c_str(), numPixels, 0.5, numPixels + 0.5);
      h3->SetOption("LIVE");
      m_particleChannelHits.push_back(h3);

      string name4 = str(format("particle_hits_per_event%1%") % (module));
      string title4 = str(format("Number of particle hits per event of module #%1%") % (module));
      TH1F* h4 = new TH1F(name4.c_str(), title4.c_str(), 50, 0, 50);
      h4->SetOption("LIVE");
      m_particleHitsPerEvent.push_back(h4);

      string name5 = str(format("other_hits_per_event%1%") % (module));
      string title5 = str(format("Number of other hits per event of module #%1%") % (module));
      TH1F* h5 = new TH1F(name5.c_str(), title5.c_str(), 50, 0, 50);
      h5->SetOption("LIVE");
      m_otherHitsPerEvent.push_back(h5);
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
    m_particleHits->Reset();
    m_otherHits->Reset();

    for (int i = 0; i < m_numModules; i++) {
      m_allHitsXY[i]->Reset();
      m_allTdc[i]->Reset();
      m_particleChannelHits[i]->Reset();
    }
  }

  void TOPDQMModule::event()
  {

    int n_particle[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int n_other[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    for (const auto& digit : m_digits) {
      int i = digit.getModuleID() - 1;
      if (i < 0 || i >= m_numModules) {
        B2ERROR("Invalid module ID found in TOPDigits: ID = " << i + 1);
        continue;
      }

      m_allHitsXY[i]->Fill(digit.getPixelCol(), digit.getPixelRow());
      m_allTdc[i]->Fill(digit.getRawTime());
      double ph = digit.getPulseHeight();
      double pw = digit.getPulseWidth();
      if (ph > m_ADCCutLow && ph < m_ADCCutHigh && pw > m_PulseWidthCutLow && pw < m_PulseWidthCutHigh) { // particle hits
        m_particleHits->Fill(i + 1);
        m_particleChannelHits[i]->Fill(digit.getPixelID());
        n_particle[i]++;
      } else { // background hits
        m_otherHits->Fill(i + 1);
        n_other[i]++;
      }
    }

    for (int i = 0; i < 16; i++) {
      m_particleHitsPerEvent[i]->Fill(n_particle[i]);
      m_otherHitsPerEvent[i]->Fill(n_other[i]);
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

