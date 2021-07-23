/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPBunchFinder/TOPBunchFinderModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/PDF1Dim.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

// Dataobject classes
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPBunchFinder)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPBunchFinderModule::TOPBunchFinderModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("A precise event T0 determination w.r.t local time reference of TOP counter. "
                   "Results available in TOPRecBunch.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("numBins", m_numBins, "number of bins of fine search region", 200);
    addParam("timeRangeFine", m_timeRangeFine,
             "time range in which to do fine search [ns]", 10.0);
    addParam("timeRangeCoarse", m_timeRangeCoarse,
             "time range in which to do coarse search, if autoRange turned off [ns]", 100.0);
    addParam("autoRange", m_autoRange,
             "turn on/off automatic determination of the coarse search region (false means off)", false);
    addParam("sigmaSmear", m_sigmaSmear,
             "sigma in [ns] for additional smearing of PDF", 0.0);
    addParam("minSignal", m_minSignal,
             "minimal number of signal photons to accept track", 10.0);
    addParam("minSBRatio", m_minSBRatio,
             "minimal signal-to-background ratio to accept track", 0.0);
    addParam("minDERatio", m_minDERatio,
             "minimal ratio of detected-to-expected photons to accept track", 0.2);
    addParam("maxDERatio", m_maxDERatio,
             "maximal ratio of detected-to-expected photons to accept track", 2.5);
    addParam("minPt", m_minPt, "minimal p_T of the track", 0.3);
    addParam("maxPt", m_maxPt, "maximal p_T of the track", 6.0);
    addParam("maxD0", m_maxD0, "maximal absolute value of helix perigee distance", 2.0);
    addParam("maxZ0", m_maxZ0, "maximal absolute value of helix perigee z coordinate", 4.0);
    addParam("minNHitsCDC", m_minNHitsCDC, "minimal number of hits in CDC", 20);
    addParam("useMCTruth", m_useMCTruth,
             "if true, use MC truth for particle mass instead of the most probable from dEdx",
             false);
    addParam("saveHistograms", m_saveHistograms,
             "if true, save histograms to TOPRecBunch", false);
    addParam("tau", m_tau,
             "first order filter time constant [number of events]", 100.0);
    addParam("fineSearch", m_fineSearch,
             "if true, do fine search with two-dimensional PDF", true);
    addParam("correctDigits", m_correctDigits,
             "if true, subtract bunch time in TOPDigits", true);
    addParam("subtractRunningOffset", m_subtractRunningOffset,
             "if true and correctDigits = True, subtract running offset in TOPDigits "
             "when running in HLT mode. It must be set to false when running calibration.",
             true);
    addParam("bunchesPerSSTclk", m_bunchesPerSSTclk,
             "number of bunches per SST clock period", 24);
    addParam("usePIDLikelihoods", m_usePIDLikelihoods,
             "use PIDLikelihoods instead of DedxLikelihoods (only if run on cdst files)",
             false);
    addParam("dedxMomentumLimit", m_dedxMomentumLimit, "momentum limit of good dEdx particle ID", 0.7);

  }


  void TOPBunchFinderModule::initialize()
  {
    // input collections

    m_topDigits.isRequired();
    m_topRawDigits.isOptional();
    m_tracks.isRequired();
    StoreArray<ExtHit> extHits;
    extHits.isRequired();
    m_initialParticles.isOptional();

    if (m_useMCTruth) {
      StoreArray<MCParticle> mcParticles;
      mcParticles.isRequired();
      StoreArray<TOPBarHit> barHits;
      barHits.isRequired();
    } else {
      if (m_usePIDLikelihoods) {
        StoreArray<PIDLikelihood> pidLikelihoods;
        pidLikelihoods.isRequired();
      } else {
        StoreArray<CDCDedxLikelihood> cdcDedxLikelihoods;
        cdcDedxLikelihoods.isRequired();
        StoreArray<VXDDedxLikelihood> vxdDedxLikelihoods;
        vxdDedxLikelihoods.isOptional();
      }
    }

    // output

    m_recBunch.registerInDataStore();
    m_timeZeros.registerInDataStore();
    m_timeZeros.registerRelationTo(extHits);
    m_eventT0.registerInDataStore(); // usually it is already registered in tracking

    // bunch separation in time

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_bunchTimeSep = geo->getNominalTDC().getSyncTimeBase() / m_bunchesPerSSTclk;

    // prior probabilities: from generic BBbar simulation
    // - MCParticles with reconstructed track and at least 5 detected Cherenkov photons

    m_priors[11] =   0.062;  // electrons
    m_priors[13] =   0.086;  // muons
    m_priors[211] =  0.734;  // pions
    m_priors[321] =  0.106;  // kaons
    m_priors[2212] = 0.013;  // protons
    m_priors[1000010020] = 0; // deuterons

    double s = 0;
    for (const auto& prior : m_priors) s += prior.second;
    for (auto& prior : m_priors) prior.second /= s;

    if (not m_commonT0.isValid()) {
      B2ERROR("Common T0 calibration payload requested but not available");
      return;
    }

    /*************************************************************************
     * auto detection of HLT/express reco mode via status of common T0 payload:
     *   c_Default           -> HLT/express reco mode
     *   c_Calibrated        -> data processing mode
     *   c_Unusable          -> HLT/express reco mode
     *   c_roughlyCalibrated -> HLT/express reco mode
     *************************************************************************/

    if (m_commonT0->isCalibrated()) {
      m_HLTmode = false;
      m_runningOffset = 0; // since digits are already commonT0 calibrated
      m_runningError = m_commonT0->getT0Error();
    } else if (m_commonT0->isRoughlyCalibrated()) {
      m_HLTmode = true;
      m_runningOffset = m_commonT0->getT0(); // since digits are not commonT0 calibrated
      m_runningError = m_commonT0->getT0Error();
    } else {
      m_HLTmode = true;
      m_runningOffset = 0;
      m_runningError = m_bunchTimeSep / sqrt(12.0);
    }

    if (m_HLTmode) {
      B2INFO("TOPBunchFinder: running in HLT/express reco mode");
    } else {
      B2INFO("TOPBunchFinder: running in data processing mode");
    }

  }


  void TOPBunchFinderModule::beginRun()
  {
    StoreObjPtr<EventMetaData> evtMetaData;

    if (not m_commonT0.isValid()) {
      B2FATAL("Common T0 calibration payload requested but not available for run "
              << evtMetaData->getRun()
              << " of experiment " << evtMetaData->getExperiment());
    }

  }


  void TOPBunchFinderModule::event()
  {

    m_processed++;
    TOPRecoManager::setDefaultTimeWindow();

    // define output for the reconstructed bunch

    if (not m_recBunch.isValid()) {
      m_recBunch.create();
    } else {
      m_recBunch->clearReconstructed();
    }
    m_timeZeros.clear();

    if (not m_eventT0.isValid()) m_eventT0.create();

    // set MC truth if available

    if (m_initialParticles.isValid()) {
      double simTime = m_initialParticles->getTime();
      int simBunchNumber = round(simTime / m_bunchTimeSep);
      m_recBunch->setSimulated(simBunchNumber, simTime);
    }

    // set revo9 counter from the first raw digit if available (all should be the same)

    if (m_topRawDigits.getEntries() > 0) {
      const auto* rawDigit = m_topRawDigits[0];
      m_recBunch->setRevo9Counter(rawDigit->getRevo9Counter());
    }

    // full time window in which data are taken (smaller time window is used in reconstruction)

    const auto& tdc = TOPGeometryPar::Instance()->getGeometry()->getNominalTDC();
    double timeWindow = m_feSetting->getReadoutWindows() * tdc.getSyncTimeBase() / TOPNominalTDC::c_syncWindows;

    // counters and temporary containers

    int numTrk = 0;
    m_nodEdxCount = 0;
    std::vector<TOPTrack> topTracks;
    std::vector<PDFConstructor> pdfConstructors;
    std::vector<PDF1Dim> top1Dpdfs;
    std::vector<int> numPhotons;
    std::vector<double> assumedMasses;
    std::vector<Chi2MinimumFinder1D> finders;

    // loop over reconstructed tracks, make a selection and push to containers

    for (const auto& track : m_tracks) {
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      // track selection
      const auto* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
      if (not fitResult) {
        B2ERROR("No TrackFitResult available. Must be a bug somewhere.");
        continue;
      }
      if (fitResult->getHitPatternCDC().getNHits() < m_minNHitsCDC) continue;
      if (fabs(fitResult->getD0()) > m_maxD0) continue;
      if (fabs(fitResult->getZ0()) > m_maxZ0) continue;
      auto pt = fitResult->getTransverseMomentum();
      if (pt < m_minPt or pt > m_maxPt) continue;

      // determine most probable particle mass
      auto chargedStable = Const::pion;
      if (m_useMCTruth) {
        if (not trk.getMCParticle()) continue;
        if (not trk.getBarHit()) continue;
        chargedStable = Const::chargedStableSet.find(abs(trk.getMCParticle()->getPDG()));
        if (chargedStable == Const::invalidParticle) continue;
      } else {
        chargedStable = getMostProbable(track);
      }

      // construct PDF
      PDFConstructor pdfConstructor(trk, chargedStable, PDFConstructor::c_Rough);
      if (not pdfConstructor.isValid()) continue;
      numTrk++;

      // make PDF projection to time axis with bin size of ~0.5 ns
      PDF1Dim pdf1d(pdfConstructor, 0.5, timeWindow);
      pdfConstructor.switchOffDeltaRayPDF(); // to speed-up fine search

      // do further track selection
      double expSignal = pdf1d.getExpectedSignal() + pdf1d.getExpectedDeltaPhotons();
      double expBG = pdf1d.getExpectedBG();
      double expPhot = expSignal + expBG;
      double numPhot = pdf1d.getNumOfPhotons();
      if (expSignal < m_minSignal) continue;
      if (expSignal < m_minSBRatio * expBG) continue;
      if (numPhot < m_minDERatio * expPhot) continue;
      if (numPhot > m_maxDERatio * expPhot) continue;

      topTracks.push_back(trk);
      pdfConstructors.push_back(pdfConstructor);
      top1Dpdfs.push_back(pdf1d);
      numPhotons.push_back(numPhot);
      assumedMasses.push_back(pdfConstructors.back().getHypothesis().getMass());
    }
    m_recBunch->setNumTracks(numTrk, topTracks.size(), m_nodEdxCount);
    if (topTracks.empty()) return;

    // set time region for coarse search

    double minT0 = -m_timeRangeCoarse / 2;
    double maxT0 = m_timeRangeCoarse / 2;
    if (m_autoRange) {
      minT0 = top1Dpdfs[0].getMinT0();
      maxT0 = top1Dpdfs[0].getMaxT0();
      for (const auto& pdf : top1Dpdfs) {
        minT0 = std::min(minT0, pdf.getMinT0());
        maxT0 = std::max(maxT0, pdf.getMaxT0());
      }
    }
    double binSize = top1Dpdfs[0].getBinSize();
    int numBins = (maxT0 - minT0) / binSize;
    maxT0 = minT0 + binSize * numBins;

    // find coarse T0

    for (const auto& pdf : top1Dpdfs) {
      finders.push_back(Chi2MinimumFinder1D(numBins, minT0, maxT0));
      auto& finder = finders.back();
      const auto& bins = finder.getBinCenters();
      for (unsigned i = 0; i < bins.size(); i++) {
        double t0 = bins[i];
        finder.add(i, -2 * pdf.getLogL(t0));
      }
    }
    auto coarseFinder = finders[0];
    for (size_t i = 1; i < finders.size(); i++) {
      coarseFinder.add(finders[i]);
    }

    const auto& t0Coarse = coarseFinder.getMinimum();
    if (m_saveHistograms) {
      m_recBunch->addHistogram(coarseFinder.getHistogram("chi2_coarse_",
                                                         "coarse T0; t_{0} [ns]; -2 log L"));
    }
    if (t0Coarse.position < minT0 or t0Coarse.position > maxT0 or not t0Coarse.valid) {
      B2DEBUG(100, "Coarse T0 finder: returning invalid or out of range T0");
      return;
    }

    auto T0 = t0Coarse;

    // find precise T0

    if (m_fineSearch) {
      finders.clear();

      double timeMin = TOPRecoManager::getMinTime() + t0Coarse.position;
      double timeMax = TOPRecoManager::getMaxTime() + t0Coarse.position;
      double t0min = t0Coarse.position - m_timeRangeFine / 2;
      double t0max = t0Coarse.position + m_timeRangeFine / 2;

      for (size_t itrk = 0; itrk < topTracks.size(); itrk++) {
        finders.push_back(Chi2MinimumFinder1D(m_numBins, t0min, t0max));
        const auto& reco = pdfConstructors[itrk];
        numPhotons[itrk] = setFinder(finders.back(), reco, timeMin, timeMax);
        const auto& trk = topTracks[itrk];
        if (trk.getMomentumMag() > m_dedxMomentumLimit) {
          std::vector<Const::ChargedStable> other;
          if (reco.getHypothesis() == Const::kaon) {
            other.push_back(Const::pion);
            other.push_back(Const::proton);
          } else if (reco.getHypothesis() == Const::proton) {
            other.push_back(Const::pion);
            other.push_back(Const::kaon);
          } else {
            other.push_back(Const::kaon);
            other.push_back(Const::proton);
          }
          for (const auto& chargedStable : other) {
            PDFConstructor pdfConstructor(trk, chargedStable, PDFConstructor::c_Rough);
            if (not pdfConstructor.isValid() or pdfConstructor.getExpectedSignalPhotons() == 0) continue;
            pdfConstructor.switchOffDeltaRayPDF(); // to speed-up fine search
            double expSignal = pdfConstructor.getExpectedSignalPhotons() + pdfConstructor.getExpectedDeltaPhotons();
            if (expSignal < m_minSignal) continue;
            Chi2MinimumFinder1D finder(m_numBins, t0min, t0max);
            int numPhot = setFinder(finder, pdfConstructor, timeMin, timeMax);
            if (numPhot != numPhotons[itrk])
              B2ERROR("Different number of photons used for log likelihood of different mass hypotheses");
            if (finder.getMinChi2() < finders.back().getMinChi2()) {
              finders.back() = finder;
              assumedMasses[itrk] = chargedStable.getMass();
            }
          }
        }
      }

      if (finders.size() == 0) return; // just in case
      auto finder = finders[0];
      for (size_t i = 1; i < finders.size(); i++) {
        finder.add(finders[i]);
      }

      const auto& t0Fine = finder.getMinimum();
      if (m_saveHistograms) {
        m_recBunch->addHistogram(finder.getHistogram("chi2_fine_", "precise T0; t_{0} [ns]; -2 log L"));
      }
      if (t0Fine.position < t0min or t0Fine.position > t0max or not t0Fine.valid) {
        B2DEBUG(100, "Fine T0 finder: returning invalid or out of range T0");
        return;
      }

      T0 = t0Fine;
    }

    // bunch time and current offset

    int bunchNo = lround(T0.position / m_bunchTimeSep); // round to nearest integer
    double offset = T0.position - m_bunchTimeSep * bunchNo;
    if (not m_commonT0->isCalibrated()) { // auto set offset range
      double deltaOffset = offset - m_runningOffset;
      if (fabs(deltaOffset + m_bunchTimeSep) < fabs(deltaOffset)) {
        offset += m_bunchTimeSep;
        bunchNo--;
      } else if (fabs(deltaOffset - m_bunchTimeSep) < fabs(deltaOffset)) {
        offset -= m_bunchTimeSep;
        bunchNo++;
      }
    }
    double error = T0.error;

    // averaging with first order filter (with adoptable time constant)

    double tau = 10 + m_success / 2;  // empirically with toy MC
    if (tau > m_tau) tau = m_tau;
    double a = exp(-1.0 / tau);
    m_runningOffset = a * m_runningOffset + (1 - a) * offset;
    double err1 = a * m_runningError;
    double err2 = (1 - a) * error;
    m_runningError = sqrt(err1 * err1 + err2 * err2);

    // store the results

    double bunchTime = bunchNo * m_bunchTimeSep;
    m_recBunch->setReconstructed(bunchNo, bunchTime, offset, error,
                                 m_runningOffset, m_runningError, m_fineSearch);
    m_recBunch->setMinChi2(T0.chi2);
    m_eventT0->addTemporaryEventT0(EventT0::EventT0Component(bunchTime, error,
                                                             Const::TOP, "bunchFinder"));
    m_success++;

    // store T0 of single tracks relative to bunchTime

    if (finders.size() == topTracks.size()) {
      for (size_t itrk = 0; itrk < topTracks.size(); itrk++) {
        const auto& trk = topTracks[itrk];
        auto& finder = finders[itrk];
        const auto& t0trk = finder.getMinimum();
        auto* timeZero = m_timeZeros.appendNew(trk.getModuleID(),
                                               t0trk.position - bunchTime,
                                               t0trk.error, numPhotons[itrk]);
        timeZero->setAssumedMass(assumedMasses[itrk]);
        if (not t0trk.valid) timeZero->setInvalid();
        timeZero->setMinChi2(finder.getMinChi2());
        timeZero->addRelationTo(trk.getExtHit());

        if (m_saveHistograms) {
          std::string num = std::to_string(itrk);
          auto chi2 = finder.getHistogram("chi2_" + num,
                                          "precise T0 single track; t_{0} [ns]; -2 log L");
          auto pdf = top1Dpdfs[itrk].getHistogram("pdf1D_" + num,
                                                  "PDF projected to time axis; time [ns]");
          TH1F hits(("hits_" + num).c_str(),
                    "time distribution of hits (t0-subtracted); time [ns]",
                    pdf.GetNbinsX(), pdf.GetXaxis()->GetXmin(), pdf.GetXaxis()->GetXmax());
          for (const auto& hit : trk.getSelectedHits()) {
            hits.Fill(hit.time - t0trk.position);
          }
          timeZero->setHistograms(chi2, pdf, hits);
        }
      }
    }

    // correct time in TOPDigits

    if (m_correctDigits) {
      for (auto& digit : m_topDigits) {
        digit.subtractT0(bunchTime);
        digit.addStatus(TOPDigit::c_EventT0Subtracted);
        if (m_HLTmode and m_subtractRunningOffset) {
          digit.subtractT0(m_runningOffset);
          double err = digit.getTimeError();
          digit.setTimeError(sqrt(err * err + m_runningError * m_runningError));
          digit.addStatus(TOPDigit::c_BunchOffsetSubtracted);
        }
      }
    }

  }


  void TOPBunchFinderModule::terminate()
  {
    B2RESULT("TOPBunchFinder: event T0 determined for " << m_success << "/"
             << m_processed << " events");
  }


  Const::ChargedStable TOPBunchFinderModule::getMostProbable(const Track& track)
  {

    std::vector<double> logL;
    std::vector<double> priors;

    if (m_usePIDLikelihoods) {
      const auto* pid = track.getRelated<PIDLikelihood>();
      if (not pid) {
        m_nodEdxCount++;
        return Const::pion;
      }
      auto subset = Const::PIDDetectorSet(Const::SVD);
      subset += Const::PIDDetectorSet(Const::CDC);
      for (const auto& type : Const::chargedStableSet) {
        logL.push_back(pid->getLogL(type, subset));
        priors.push_back(m_priors[abs(type.getPDGCode())]);
      }
    } else {
      const auto* cdcdedx = track.getRelated<CDCDedxLikelihood>();
      const auto* vxddedx = track.getRelated<VXDDedxLikelihood>();
      if (not cdcdedx and not vxddedx) {
        m_nodEdxCount++;
        return Const::pion;
      }
      for (const auto& type : Const::chargedStableSet) {
        if (cdcdedx and vxddedx) {
          logL.push_back(cdcdedx->getLogL(type) + vxddedx->getLogL(type));
        } else if (cdcdedx) {
          logL.push_back(cdcdedx->getLogL(type));
        } else {
          logL.push_back(vxddedx->getLogL(type));
        }
        priors.push_back(m_priors[abs(type.getPDGCode())]);
      }
    }

    // get maximal logL
    auto logL_max = logL[0];
    for (auto x : logL) {
      if (x > logL_max) logL_max = x;
    }

    // calculate probabilities, normalizaton is not needed
    std::vector<double> probability(logL.size());
    for (unsigned i = 0; i < logL.size(); ++i) {
      probability[i] = exp(logL[i] - logL_max) * priors[i];
    }

    // find most probable
    unsigned i0 = 0;
    for (unsigned i = 0; i < probability.size(); ++i) {
      if (probability[i] > probability[i0]) i0 = i;
    }
    return Const::chargedStableSet.at(i0);

  }


  int TOPBunchFinderModule::setFinder(Chi2MinimumFinder1D& finder, const PDFConstructor& reco, double timeMin, double timeMax)
  {
    std::set<int> nfotSet; // for control only
    const auto& binCenters = finder.getBinCenters();
    int numPhotons = 0;
    double logL_bkg = reco.getBackgroundLogL(timeMin, timeMax).logL;
    for (unsigned i = 0; i < binCenters.size(); i++) {
      double t0 = binCenters[i];
      auto LL = reco.getLogL(t0, timeMin, timeMax, m_sigmaSmear);
      finder.add(i, -2 * (LL.logL - logL_bkg));
      if (i == 0) numPhotons = LL.numPhotons;
      nfotSet.insert(LL.numPhotons);
    }

    if (nfotSet.size() != 1) B2ERROR("Different number of photons used for log likelihood of different time shifts");

    return numPhotons;
  }

} // end Belle2 namespace
