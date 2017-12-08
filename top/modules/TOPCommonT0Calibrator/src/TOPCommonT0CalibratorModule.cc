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
#include <top/modules/TOPCommonT0Calibrator/TOPCommonT0CalibratorModule.h>
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

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dbobjects/TOPCalCommonT0.h>

// Root
#include "TFile.h"


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
    // set module description (e.g. insert text)
    setDescription("On-line common T0 calibrator (under development!)");
    //    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("numBins", m_numBins, "number of bins", 200);
    addParam("timeRange", m_timeRange, "time range in which to search [ns]", 1.0);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use full TDC range)", 51.2);
    addParam("numEvents", m_numEvents, "number of events to merge", 100);

  }

  TOPCommonT0CalibratorModule::~TOPCommonT0CalibratorModule()
  {
  }

  void TOPCommonT0CalibratorModule::initialize()
  {
    m_logLikelihood.resize(m_numBins);
    m_dt = m_timeRange / m_numBins;
    for (int i = 0; i < m_numBins; i++) {
      m_t0.push_back((2 * i - m_numBins + 1) * m_dt / 2.0);
    }

    // input

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    // output

    StoreArray<TOPCalCommonT0> commonT0;
    commonT0.registerInDataStore();

    // Configure TOP detector

    TOPconfigure config;
    if (m_maxTime <= 0) m_maxTime = config.getTDCTimeRange();


  }

  void TOPCommonT0CalibratorModule::beginRun()
  {
  }

  void TOPCommonT0CalibratorModule::event()
  {

    // create reconstruction object and set various options

    int Nhyp = 1;
    double mass = Const::muon.getMass();
    TOPreco reco(Nhyp, &mass);
    reco.setPDFoption(TOPreco::c_Rough);
    reco.setTmax(m_maxTime + m_timeRange / 2);

    // add photon hits to reconstruction object

    StoreArray<TOPDigit> topDigits;
    for (const auto& digit : topDigits) {
      if (digit.getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                     digit.getTimeError());
    }

    // loop over reconstructed tracks and make sum of log likelihoods for diff. bunches
    std::vector<TOPtrack> topTracks;
    StoreArray<Track> tracks;
    for (const auto& track : tracks) {
      TOPtrack trk(&track);
      if (trk.getP() < 3) continue;
      if (abs(trk.getPDGcode()) != 13) continue;
      if (trk.isValid()) topTracks.push_back(trk);
    }
    if (topTracks.size() != 2) return;

    for (auto& trk : topTracks) {
      reco.reconstruct(trk);
      if (reco.getFlag() != 1) return;

      for (int i = 0; i < m_numBins; i++) {
        m_logLikelihood[i] += reco.getLogL(m_t0[i], m_maxTime);
      }
    }
    m_iEvent++;
    if (m_iEvent < m_numEvents) return;

    /*
    if(m_histograms.size() < 10) {
      std::string name = "h" + std::to_string(m_histograms.size());
      TH1F* h = new TH1F(name.c_str(), "di-muon log Likelihood vs. t0",
           m_numBins, -m_timeRange / 2, m_timeRange / 2);
      m_histograms.push_back(h);
      h->GetXaxis()->SetTitle("t_{0} [ns]");
      h->GetYaxis()->SetTitle("log L_{#mu+} + log L_{#mu-}");
      for(int i = 0; i < m_numBins; i++) h->SetBinContent(i + 1, m_logLikelihood[i]);
    }
    */


    // find maximum

    int i0 = 0;
    for (int i = 0; i < m_numBins; i++) {
      if (m_logLikelihood[i] > m_logLikelihood[i0]) i0 = i;
    }

    PointWithError t0 = getParabolicMaximum(i0);

    StoreArray<TOPCalCommonT0> commonT0;
    commonT0.appendNew(t0.value, t0.error);

    // reset
    for (auto& logL : m_logLikelihood) logL = 0;
    m_iEvent = 0;

  }


  void TOPCommonT0CalibratorModule::endRun()
  {
  }

  void TOPCommonT0CalibratorModule::terminate()
  {
    if (!m_histograms.empty()) {
      TFile* file = new TFile("commonT0Histo.root", "RECREATE");
      for (auto& h : m_histograms) h->Write();
      file->Close();
    }

  }


  PointWithError TOPCommonT0CalibratorModule::getParabolicMaximum(unsigned i0)
  {
    if (i0 == 0) i0++;
    if (i0 == m_t0.size() - 1) i0--;

    PointWithError x = getParabolicMaximum(m_logLikelihood[i0 - 1],
                                           m_logLikelihood[i0],
                                           m_logLikelihood[i0 + 1]);
    return PointWithError(m_t0[i0] + x.value * m_dt, x.error * m_dt);
  }


  PointWithError TOPCommonT0CalibratorModule::getParabolicMaximum(double yLeft,
      double yCenter,
      double yRight)
  {
    double D21 = yCenter - yLeft;
    double D32 = yRight - yCenter;
    double A = (D32 - D21) / 2;
    if (A == 0) return PointWithError(0, 0);
    double B = (D32 + D21) / 2;
    return PointWithError(- B / 2 / A, sqrt(-2 / A)); // TODO: check error calculation!!
  }


} // end Belle2 namespace

