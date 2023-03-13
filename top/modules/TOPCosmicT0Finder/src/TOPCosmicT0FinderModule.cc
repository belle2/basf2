/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <top/modules/TOPCosmicT0Finder/TOPCosmicT0FinderModule.h>

// TOP headers.
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/PDF1Dim.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPTimeZero.h>

// Root
#include <TH1F.h>

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCosmicT0Finder);

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
    addParam("minHits", m_minHits,
             "minimal number of detected photons in a module", (unsigned) 10);
    addParam("minSignal", m_minSignal,
             "minimal number of expected signal photons", 5.0);
    addParam("applyT0", m_applyT0, "if true, subtract T0 in TOPDigits", true);
    addParam("numBins", m_numBins, "number of bins time range is divided to", 200);
    addParam("timeRange", m_timeRange, "time range in which to search [ns]", 10.0);
    addParam("sigma", m_sigma, "additional time spread added to PDF [ns]", 0.0);
    addParam("saveHistograms", m_saveHistograms,
             "save histograms to TOPTimeZero", false);
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

    StoreArray<TOPTimeZero> timeZeros;
    timeZeros.registerInDataStore();
    timeZeros.registerRelationTo(tracks);
    timeZeros.registerRelationTo(extHits);
    timeZeros.registerRelationTo(barHits);
  }

  void TOPCosmicT0FinderModule::event()
  {
    TOPRecoManager::setDefaultTimeWindow();

    // select track: if several, choose highest momentum track

    StoreArray<Track> tracks;
    const ExtHit* selectedExtHit = 0;
    double p0 = 0;
    for (const auto& track : tracks) {
      const auto extHits = track.getRelationsWith<ExtHit>();
      const ExtHit* extHit0 = 0;
      for (const auto& extHit : extHits) {
        if (abs(extHit.getPdgCode()) != Const::muon.getPDGCode()) continue;
        if (extHit.getDetectorID() != Const::EDetector::TOP) continue;
        if (extHit.getCopyID() <= 0) continue;
        double dot = extHit.getPosition().Dot(extHit.getMomentum());
        if (m_useIncomingTrack) {
          if (dot > 0) continue;
          if (not extHit0) extHit0 = &extHit;
          if (extHit.getTOF() < extHit0->getTOF()) extHit0 = &extHit;
        } else {
          if (dot < 0) continue;
          if (not extHit0) extHit0 = &extHit;
          if (extHit.getTOF() > extHit0->getTOF()) extHit0 = &extHit;
        }
      }
      if (not extHit0) continue;
      double p = extHit0->getMomentum().R();
      if (p > p0) {
        p0 = p;
        selectedExtHit = extHit0;
      }
    }
    if (not selectedExtHit) return;

    TOPTrack topTrack(selectedExtHit);
    if (not topTrack.isValid()) return;

    // require minimal number of photon hits

    if (topTrack.getSelectedHits().size() < m_minHits) return;

    // construct PDF for muon

    PDFConstructor pdfConstructor(topTrack, Const::muon, PDFConstructor::c_Rough);
    if (not pdfConstructor.isValid()) return;

    // require minimal expected signal

    if (pdfConstructor.getExpectedSignalPhotons() < m_minSignal) return;

    // event is accepted for t0 determination

    m_acceptedCount++;

    // full time window in which data are taken (smaller time window is used in reconstruction)

    const auto& tdc = TOPGeometryPar::Instance()->getGeometry()->getNominalTDC();
    double timeWindow = m_feSetting->getReadoutWindows() * tdc.getSyncTimeBase() / static_cast<double>(TOPNominalTDC::c_syncWindows);

    // find rough t0

    PDF1Dim pdf1d(pdfConstructor, 1.0, timeWindow);  // ~1 ns bin size
    pdfConstructor.switchOffDeltaRayPDF(); // to speed-up fine search

    Chi2MinimumFinder1D roughFinder(pdf1d.getNumBinsT0(), pdf1d.getMinT0(), pdf1d.getMaxT0());
    const auto& bins = roughFinder.getBinCenters();
    for (unsigned i = 0; i < bins.size(); i++) {
      double t0 = bins[i];
      roughFinder.add(i, -2 * pdf1d.getLogL(t0));
    }
    const auto& t0Rough = roughFinder.getMinimum();

    // find precise t0

    double timeMin = TOPRecoManager::getMinTime() + t0Rough.position;
    double timeMax = TOPRecoManager::getMaxTime() + t0Rough.position;
    double t0min = t0Rough.position - m_timeRange / 2;
    double t0max = t0Rough.position + m_timeRange / 2;
    Chi2MinimumFinder1D finder(m_numBins, t0min, t0max);
    const auto& binCenters = finder.getBinCenters();
    int numPhotons = 0;
    for (unsigned i = 0; i < binCenters.size(); i++) {
      double t0 = binCenters[i];
      auto LL = pdfConstructor.getLogL(t0, timeMin, timeMax, m_sigma);
      finder.add(i, -2 * LL.logL);
      if (i == 0) numPhotons = LL.numPhotons;
    }
    const auto& t0 = finder.getMinimum();
    if (t0.position < t0min or t0.position > t0max or not t0.valid) return; // out of range

    // event t0 is successfully determined

    m_successCount++;

    // store results

    StoreArray<TOPTimeZero> timeZeros;
    auto* timeZero = timeZeros.appendNew(topTrack.getModuleID(), t0.position, t0.error, numPhotons);
    timeZero->addRelationTo(topTrack.getTrack());
    timeZero->addRelationTo(topTrack.getExtHit());
    if (topTrack.getBarHit()) timeZero->addRelationTo(topTrack.getBarHit());

    // save histograms

    if (m_saveHistograms) {
      std::string name;

      name = "chi2_" + to_string(m_num);
      std::string title = "muon -2 logL vs. t0; t_{0} [ns]; -2 log L_{#mu}";
      auto chi2 = finder.getHistogram(name, title);

      name = "pdf_" + to_string(m_num);
      auto pdf = pdf1d.getHistogram(name, "PDF projected to time axis");
      pdf.SetName(name.c_str());
      pdf.SetXTitle("time [ns]");
      pdf.SetLineColor(2);
      pdf.SetOption("hist");

      name = "hits_" + to_string(m_num);
      TH1F hits(name.c_str(), "time distribution of photons (t0-subtracted)",
                pdf.GetNbinsX(), pdf.GetXaxis()->GetXmin(), pdf.GetXaxis()->GetXmax());
      hits.SetXTitle("time [ns]");
      for (const auto& hit : topTrack.getSelectedHits()) {
        hits.Fill(hit.time - t0.position);
      }
      timeZero->setHistograms(chi2, pdf, hits);
      m_num++;
    }

    // subtract T0 in digits

    if (m_applyT0) {
      StoreArray<TOPDigit> topDigits;
      for (auto& digit : topDigits) {
        digit.subtractT0(t0.position);
        double err = digit.getTimeError();
        digit.setTimeError(sqrt(err * err + t0.error * t0.error));
        digit.addStatus(TOPDigit::c_EventT0Subtracted);
      }
    }

  }


  void TOPCosmicT0FinderModule::terminate()
  {
    B2RESULT("TOPCosmicT0Finder: accepted events " << m_acceptedCount
             << ", event T0 determined for " << m_successCount << " events");
  }


} // end Belle2 namespace


