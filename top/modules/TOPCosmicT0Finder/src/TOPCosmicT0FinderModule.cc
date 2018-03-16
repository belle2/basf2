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
#include <top/reconstruction/TOP1Dpdf.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

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
    addParam("bkgPerModule", m_bkgPerModule,
             "average number of background hits per module", 1.0);
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

    StoreArray<TOPTimeZero> timeZeros;
    timeZeros.registerInDataStore();
    timeZeros.registerRelationTo(tracks);
    timeZeros.registerRelationTo(extHits);
    timeZeros.registerRelationTo(barHits);

    // Configure TOP detector

    TOPconfigure config;

  }

  void TOPCosmicT0FinderModule::beginRun()
  {
  }

  void TOPCosmicT0FinderModule::event()
  {

    // select track: if several, choose highest momentum track

    StoreArray<Track> tracks;
    const Track* selectedTrack = 0;
    double p0 = 0;
    int moduleID = 0;
    for (const auto& track : tracks) {
      const auto extHits = track.getRelationsWith<ExtHit>();
      const ExtHit* extHit0 = 0;
      for (const auto& extHit : extHits) {
        if (abs(extHit.getPdgCode()) != 13) continue;
        if (extHit.getDetectorID() != Const::EDetector::TOP) continue;
        double dot = extHit.getPosition() * extHit.getMomentum();
        if (m_useIncomingTrack) {
          if (dot > 0) continue;
          if (!extHit0) extHit0 = &extHit;
          if (extHit.getTOF() < extHit0->getTOF()) extHit0 = &extHit;
        } else {
          if (dot < 0) continue;
          if (!extHit0) extHit0 = &extHit;
          if (extHit.getTOF() > extHit0->getTOF()) extHit0 = &extHit;
        }
      }
      if (!extHit0) continue;
      double p = extHit0->getMomentum().Mag();
      if (p > p0) {
        p0 = p;
        selectedTrack = &track;
        moduleID = abs(extHit0->getCopyID());
      }
    }
    if (!selectedTrack) return;

    if (moduleID == 0) {
      B2ERROR("moduleID == 0");
      return;
    }

    TOPtrack topTrack(selectedTrack, moduleID, Const::muon);
    if (!topTrack.isValid()) return;

    // select photons: at least m_minHits

    StoreArray<TOPDigit> topDigits;
    std::vector<const TOPDigit*> selDigits;
    for (const auto& digit : topDigits) {
      if (digit.getModuleID() != topTrack.getModuleID()) continue;
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      selDigits.push_back(&digit);
    }
    if (selDigits.empty() or selDigits.size() < m_minHits) return;

    // create reconstruction object and set various options

    int Nhyp = 1;
    double mass = Const::muon.getMass();
    TOPreco reco(Nhyp, &mass, m_bkgPerModule);
    reco.setPDFoption(TOPreco::c_Rough);

    // add photon hits to reconstruction object

    for (const auto& digit : selDigits) {
      if (digit->getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit->getModuleID(), digit->getPixelID(), digit->getTime(),
                     digit->getTimeError());
    }

    // construct PDF

    reco.reconstruct(topTrack);
    if (reco.getFlag() != 1) return;
    if (reco.getExpectedPhotons() - reco.getExpectedBG() < m_minSignal) return;

    // event is accepted for t0 determination

    m_acceptedCount++;

    // find rough t0

    TOP1Dpdf pdf1d(reco, topTrack.getModuleID(), 1.0); // ~1 ns bin size
    double t0_min = pdf1d.getTminFot() - pdf1d.getTmaxPDF() - 2 * pdf1d.getBinSize();
    double t0_max = pdf1d.getTmaxFot() - pdf1d.getTminPDF();
    int numBins = std::max(int((t0_max - t0_min) / pdf1d.getBinSize()), 3);
    t0_max = t0_min + numBins * pdf1d.getBinSize();

    Chi2MinimumFinder1D roughFinder(numBins, t0_min, t0_max);
    const auto& bins = roughFinder.getBinCenters();
    for (unsigned i = 0; i < bins.size(); i++) {
      double t0 = bins[i];
      roughFinder.add(i, -2 * pdf1d.getLogL(t0));
    }
    const auto& t0Rough = roughFinder.getMinimum();

    // find precise t0

    const auto& tdc = TOPGeometryPar::Instance()->getGeometry()->getNominalTDC();
    double timeMin = tdc.getTimeMin() + t0Rough.position;
    double timeMax = tdc.getTimeMax() + t0Rough.position;
    double t0min = t0Rough.position - m_timeRange / 2;
    double t0max = t0Rough.position + m_timeRange / 2;
    Chi2MinimumFinder1D finder(m_numBins, t0min, t0max);
    const auto& binCenters = finder.getBinCenters();
    for (unsigned i = 0; i < binCenters.size(); i++) {
      double t0 = binCenters[i];
      finder.add(i, -2 * reco.getLogL(t0, timeMin, timeMax, m_sigma));
    }
    const auto& t0 = finder.getMinimum();
    if (t0.position < t0min or t0.position > t0max or !t0.valid) return; // out of range

    // event t0 is successfully determined

    m_successCount++;

    // store results

    StoreArray<TOPTimeZero> timeZeros;
    auto* timeZero = timeZeros.appendNew(topTrack.getModuleID(), t0.position, t0.error,
                                         reco.getNumOfPhotons());
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
      for (const auto& digit : selDigits) {
        if (digit->getHitQuality() == TOPDigit::c_Good)
          hits.Fill(digit->getTime() - t0.position);
      }

      timeZero->setHistograms(chi2, pdf, hits);
      m_num++;
    }

    // subtract T0 in digits

    if (m_applyT0) {
      for (auto& digit : topDigits) {
        digit.subtractT0(t0.position);
        double err = digit.getTimeError();
        digit.setTimeError(sqrt(err * err + t0.error * t0.error));
        digit.addStatus(TOPDigit::c_EventT0Subtracted);
      }
    }

  }


  void TOPCosmicT0FinderModule::endRun()
  {
  }

  void TOPCosmicT0FinderModule::terminate()
  {
    B2RESULT("TOPCosmicT0Finder: accepted events " << m_acceptedCount
             << ", event T0 determined for " << m_successCount << " events");
  }


} // end Belle2 namespace


