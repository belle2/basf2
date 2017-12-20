/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPCosmicT0Finder/TOPCosmicT0FinderModule.h>
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

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dbobjects/TOPCalCommonT0.h>

// Root
#include "TFile.h"

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCosmicT0Finder)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCosmicT0FinderModule::TOPCosmicT0FinderModule() : Module()

  {
    // set module description
    setDescription("Event T0 finder for global cosmic runs");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("useIncomingTrack", m_useIncomingTrack,
             "if true, use incoming track, otherwise use outcoming track", true);
    addParam("minCDCHits", m_minCDCHits,
             "minimal number of CDC hits of the track", (unsigned) 20);
    addParam("applyT0", m_applyT0, "if true, subtract T0 in TOPDigits", true);
    addParam("numBins", m_numBins, "number of bins", 1000);
    addParam("timeRange", m_timeRange, "time range in which to search [ns]", 40.0);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use full TDC range)", 51.2);

  }

  TOPCosmicT0FinderModule::~TOPCosmicT0FinderModule()
  {
  }

  void TOPCosmicT0FinderModule::initialize()
  {

    // input

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();

    // output

    StoreArray<TOPCalCommonT0> commonT0;
    commonT0.registerInDataStore();
    //    commonT0.registerRelationTo(tracks);
    //    commonT0.registerRelationTo(extHits);
    //    commonT0.registerRelationTo(barHits);

    // time axis

    m_dt = m_timeRange / m_numBins;
    for (int i = 0; i < m_numBins; i++) {
      m_t0.push_back((2 * i - m_numBins + 1) * m_dt / 2.0);
    }

    // Configure TOP detector

    TOPconfigure config;
    if (m_maxTime <= 0) m_maxTime = config.getTDCTimeRange();

  }

  void TOPCosmicT0FinderModule::beginRun()
  {
  }

  void TOPCosmicT0FinderModule::event()
  {

    // select track

    StoreArray<Track> tracks;
    const Track* selectedTrack = 0;
    double deltaTime = 0;
    for (const auto& track : tracks) {
      const auto* recoTrack = track.getRelated<RecoTrack>();
      if (!recoTrack) {
        B2ERROR("No related RecoTrack found");
        continue;
      }
      if (recoTrack->getNumberOfCDCHits() < m_minCDCHits) continue;

      const auto* trackRep = recoTrack->getCardinalRepresentation();
      const auto& firstState = recoTrack->getMeasuredStateOnPlaneFromFirstHit(trackRep);
      const auto& lastState = recoTrack->getMeasuredStateOnPlaneFromLastHit(trackRep);
      bool incoming = firstState.getPos().Perp() > lastState.getPos().Perp();
      if ((m_useIncomingTrack and incoming) or (!m_useIncomingTrack and !incoming)) {
        double dt = lastState.getTime() - firstState.getTime();
        if (dt > deltaTime) {
          selectedTrack = &track;
          deltaTime = dt;
        }
      }
    }
    if (!selectedTrack) return;

    TOPtrack topTrack(selectedTrack);
    if (!topTrack.isValid()) return;

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

    // calculate log likelihoods

    reco.reconstruct(topTrack);
    if (reco.getFlag() != 1) return;

    m_logLikelihoods.clear();
    for (double t0 : m_t0) m_logLikelihoods.push_back(reco.getLogL(-t0, m_maxTime));
    // TODO: consistent definition of t0 in getLogL !

    // find maximum

    int i0 = 0;
    for (int i = 0; i < m_numBins; i++) {
      if (m_logLikelihoods[i] > m_logLikelihoods[i0]) i0 = i;
    }

    PointWithError t0 = getParabolicMaximum(i0);

    StoreArray<TOPCalCommonT0> commonT0s;
    commonT0s.appendNew(t0.value, t0.error);
    //    commonT0->addRelationTo(topTrack.getTrack());
    //    commonT0->addRelationTo(topTrack.getExtHit());
    //    if(topTrack.getBarHit()) commonT0->addRelationTo(topTrack.getBarHit());

    // TODO: use debug mode here or remove ...
    cout << "t0 = " << t0.value << " +- " << t0.error << endl;
    const auto* barHit = topTrack.getBarHit();
    if (barHit) cout << "barHit.time = " << barHit->getTime()
                       << ", " << barHit->getTime() - t0.value << endl;
    const auto* extHit = topTrack.getExtHit();
    if (extHit) cout << "extHit.time = " << extHit->getTOF() << endl;
    if (barHit) cout << "delta = " << barHit->getTime() - t0.value - extHit->getTOF()
                       << ", p = " << topTrack.getP()
                       << ", nfot = " << reco.getNumOfPhotons()
                       << ", slot: " << barHit->getModuleID()
                       << " " << extHit->getCopyID() << endl;

    if (m_histograms.size() < 100) {
      std::string name = "h" + std::to_string(m_histograms.size());
      TH1F* h = new TH1F(name.c_str(), "muon log Likelihood vs. t0",
                         m_numBins, -m_timeRange / 2, m_timeRange / 2);
      m_histograms.push_back(h);
      h->GetXaxis()->SetTitle("t_{0} [ns]");
      h->GetYaxis()->SetTitle("log L_{#mu+} + log L_{#mu-}");
      for (int i = 0; i < m_numBins; i++) h->SetBinContent(i + 1, m_logLikelihoods[i]);
    }

    // subtract T0 in digits

    if (m_applyT0) {
      for (auto& digit : topDigits) {
        digit.subtractT0(t0.value);
      }
    }

  }


  void TOPCosmicT0FinderModule::endRun()
  {
  }

  void TOPCosmicT0FinderModule::terminate()
  {
    if (!m_histograms.empty()) {
      TFile* file = new TFile("commonT0Histo.root", "RECREATE");
      for (auto& h : m_histograms) h->Write();
      file->Close();
    }
  }


  PointWithError TOPCosmicT0FinderModule::getParabolicMaximum(unsigned i0)
  {
    if (i0 == 0) i0++;
    if (i0 == m_t0.size() - 1) i0--;

    PointWithError x = getParabolicMaximum(m_logLikelihoods[i0 - 1],
                                           m_logLikelihoods[i0],
                                           m_logLikelihoods[i0 + 1]);
    return PointWithError(m_t0[i0] + x.value * m_dt, x.error * m_dt);
  }


  PointWithError TOPCosmicT0FinderModule::getParabolicMaximum(double yLeft,
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

