/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPBunchFinder/TOPBunchFinderModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>
#include <top/reconstruction/TOP1Dpdf.h>
#include <top/utilities/Chi2MinimumFinder1D.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>
#include <reconstruction/dataobjects/VXDDedxLikelihood.h>
#include <top/dataobjects/TOPRecBunch.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

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

  TOPBunchFinderModule::TOPBunchFinderModule() : Module(),
    m_bunchTimeSep(0)

  {
    // set module description (e.g. insert text)
    setDescription("A precise event T0 determination w.r.t local time reference of TOP counter. Results available in TOPRecBunch.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("numBins", m_numBins, "number of bins of fine search region", 200);
    addParam("timeRange", m_timeRange,
             "time range in which to do fine search [ns]", 10.0);
    addParam("sigmaSmear", m_sigmaSmear,
             "sigma in [ns] for additional smearing of PDF", 0.0);
    addParam("minSignal", m_minSignal,
             "minimal number of signal photons to accept track", 10.0);
    addParam("minSBRatio", m_minSBRatio,
             "minimal signal-to-background ratio to accept track", 0.0);
    addParam("minDERatio", m_minDERatio,
             "minimal ratio of detected-to-expected photons to accept track", 0.4);
    addParam("maxDERatio", m_maxDERatio,
             "maximal ratio of detected-to-expected photons to accept track", 2.5);
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
    addParam("addOffset", m_addOffset,
             "if true, add running average offset to bunch time. "
             "To be used when common T0 calibration is not available (HLT, express reco)",
             false);
    addParam("bias", m_bias,
             "bias in bunch time determination [ns], to be subtracted", 0.0);
    addParam("bunchesPerSSTclk", m_bunchesPerSSTclk,
             "number of bunches per SST clock period", 12);
  }


  void TOPBunchFinderModule::initialize()
  {
    // input collections

    m_topDigits.isRequired();
    m_topRawDigits.isOptional();
    m_tracks.isRequired();
    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    if (m_useMCTruth) {
      StoreArray<MCParticle> mcParticles;
      mcParticles.isRequired();
      StoreArray<TOPBarHit> barHits;
      barHits.isRequired();
    } else {
      StoreArray<CDCDedxLikelihood> cdcDedxLikelihoods;
      cdcDedxLikelihoods.isRequired();
      StoreArray<VXDDedxLikelihood> vxdDedxLikelihoods;
      vxdDedxLikelihoods.isOptional();
    }

    // output

    m_recBunch.registerInDataStore();

    // Configure TOP detector for reconstruction

    TOPconfigure config;

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

  }


  void TOPBunchFinderModule::event()
  {

    m_processed++;

    // define output for reconstructed bunch values

    if (!m_recBunch.isValid()) m_recBunch.create();

    // set revo9 counter from the first raw digit (all should be the same)

    if (m_topRawDigits.getEntries() > 0) {
      const auto* rawDigit = m_topRawDigits[0];
      m_recBunch->setRevo9Counter(rawDigit->getRevo9Counter());
    }

    // create reconstruction object and set various options

    int Nhyp = 1;
    double pionMass = Const::pion.getMass();
    TOPreco reco(Nhyp, &pionMass);
    reco.setPDFoption(TOPreco::c_Rough);

    // add photon hits to reconstruction object

    for (const auto& digit : m_topDigits) {
      if (digit.getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                     digit.getTimeError());
    }

    // counters and temporary containers

    int numTrk = 0;
    int nodEdxTrk = 0;
    std::vector<TOPtrack> topTracks;
    std::vector<double> masses;
    std::vector<TOP1Dpdf> top1Dpdfs;

    // loop over reconstructed tracks, make a selection and push to containers

    for (const auto& track : m_tracks) {
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // determine most probable particle mass
      double mass = Const::pion.getMass();
      bool nodEdx = false;
      if (m_useMCTruth) {
        if (!trk.getMCParticle()) continue;
        if (!trk.getBarHit()) continue;
        mass = trk.getMCParticle()->getMass();
      } else {
        auto cdcdedx = track.getRelated<CDCDedxLikelihood>();
        auto vxddedx = track.getRelated<VXDDedxLikelihood>();
        if (cdcdedx or vxddedx) {
          mass = getMostProbableMass(cdcdedx, vxddedx);
        } else {
          nodEdx = true;
          B2DEBUG(100, "TOPBunchFinder: track has no relation to DedxLikelihoods - "
                  "pion mass used instead");
        }
      }

      // reconstruct (e.g. set PDF internally)
      reco.setMass(mass);
      reco.reconstruct(trk);
      if (reco.getFlag() != 1) continue; // track is not in the acceptance of TOP
      numTrk++;

      // one dimensional PDF with bin size of ~0.5 ns
      TOP1Dpdf pdf1d(reco, m_topDigits, trk.getModuleID(), 0.5);

      // do further track selection
      double expSignal = pdf1d.getExpectedSignal();
      double expBG = pdf1d.getExpectedBG();
      double expPhot = expSignal + expBG;
      double numPhot = pdf1d.getNumOfPhotons();
      if (expSignal < m_minSignal) continue;
      if (expSignal < m_minSBRatio * expBG) continue;
      if (numPhot < m_minDERatio * expPhot) continue;
      if (numPhot > m_maxDERatio * expPhot) continue;

      topTracks.push_back(trk);
      masses.push_back(mass);
      if (nodEdx) nodEdxTrk++;
      top1Dpdfs.push_back(pdf1d);

    }
    m_recBunch->setNumTracks(numTrk, topTracks.size(), nodEdxTrk);
    if (topTracks.empty()) return;

    // determine search region

    double minT0 = top1Dpdfs[0].getMinT0();
    double maxT0 = top1Dpdfs[0].getMaxT0();
    double binSize = top1Dpdfs[0].getBinSize();
    for (const auto& pdf : top1Dpdfs) {
      minT0 = std::min(minT0, pdf.getMinT0());
      maxT0 = std::max(maxT0, pdf.getMaxT0());
    }
    int numBins = (maxT0 - minT0) / binSize;
    maxT0 = minT0 + binSize * numBins;

    // find rough T0

    Chi2MinimumFinder1D roughFinder(numBins, minT0, maxT0);
    for (const auto& pdf : top1Dpdfs) {
      const auto& bins = roughFinder.getBinCenters();
      for (unsigned i = 0; i < bins.size(); i++) {
        double t0 = bins[i];
        roughFinder.add(i, -2 * pdf.getLogL(t0));
      }
    }
    const auto& t0Rough = roughFinder.getMinimum();
    if (m_saveHistograms) {
      m_recBunch->addHistogram(roughFinder.getHistogram("chi2_rough_",
                                                        "rough T0; t_{0} [ns]; -2 log L"));
    }
    if (t0Rough.position < minT0 or t0Rough.position > maxT0 or !t0Rough.valid) {
      B2WARNING("rough t0: invalid or out of range!");
      return;
    }

    auto T0 = t0Rough;

    // find precise T0

    if (m_fineSearch) {

      const auto& tdc = TOPGeometryPar::Instance()->getGeometry()->getNominalTDC();
      double timeMin = tdc.getTimeMin() + t0Rough.position;
      double timeMax = tdc.getTimeMax() + t0Rough.position;
      double t0min = t0Rough.position - m_timeRange / 2;
      double t0max = t0Rough.position + m_timeRange / 2;
      Chi2MinimumFinder1D finder(m_numBins, t0min, t0max);

      for (size_t itrk = 0; itrk < topTracks.size(); itrk++) {
        auto& trk = topTracks[itrk];
        auto mass = masses[itrk];
        reco.setMass(mass);
        reco.reconstruct(trk);
        if (reco.getFlag() != 1) {
          B2ERROR("TOPBunchFinder: track is not in the acceptance -> must be a bug");
          continue;
        }
        const auto& binCenters = finder.getBinCenters();
        for (unsigned i = 0; i < binCenters.size(); i++) {
          double t0 = binCenters[i];
          finder.add(i, -2 * reco.getLogL(t0, timeMin, timeMax, m_sigmaSmear));
        }
      }
      const auto& t0Fine = finder.getMinimum();
      if (m_saveHistograms) {
        m_recBunch->addHistogram(finder.getHistogram("chi2_fine_",
                                                     "precise T0; t_{0} [ns]; -2 log L"));
      }
      if (t0Fine.position < t0min or t0Fine.position > t0max or !t0Fine.valid) {
        B2WARNING("fine t0: invalid or out of range!");
        return;
      }

      T0 = t0Fine;
    }

    // subtract bias

    T0.position -= m_bias;

    // bunch time and current offset

    int bunchNo = lround(T0.position / m_bunchTimeSep);
    double offset = T0.position - m_bunchTimeSep * bunchNo;
    double deltaOffset = offset - m_offset;
    if (fabs(deltaOffset + m_bunchTimeSep) < fabs(deltaOffset)) {
      offset += m_bunchTimeSep;
      bunchNo--;
    } else if (fabs(deltaOffset - m_bunchTimeSep) < fabs(deltaOffset)) {
      offset -= m_bunchTimeSep;
      bunchNo++;
    }
    double error = T0.error;

    if (m_eventCount == 0) {
      m_offset = offset;
      m_error = error;
    }
    m_eventCount++;

    // averaging with first order filter

    double a = exp(-1.0 / m_tau);
    m_offset = a * m_offset + (1 - a) * offset;
    double err1 = a * m_error;
    double err2 = (1 - a) * error;
    m_error = sqrt(err1 * err1 + err2 * err2);

    // store the results

    double bunchTime = bunchNo * m_bunchTimeSep;
    if (m_addOffset) bunchTime += m_offset;

    m_recBunch->setReconstructed(bunchNo, bunchTime, offset, error, m_offset, m_error,
                                 m_fineSearch);
    m_success++;

    // correct time in TOPDigits

    if (m_correctDigits) {
      for (auto& digit : m_topDigits) {
        digit.subtractT0(bunchTime);
        digit.addStatus(TOPDigit::c_EventT0Subtracted);
        if (m_addOffset) {
          double err = digit.getTimeError();
          digit.setTimeError(sqrt(err * err + m_error * m_error));
          digit.addStatus(TOPDigit::c_OffsetSubtracted);
        }
      }
    }

  }


  void TOPBunchFinderModule::terminate()
  {
    B2RESULT("TOPBunchFinder: event T0 determined for " << m_success << "/"
             << m_processed << " events");
  }


  double TOPBunchFinderModule::getMostProbableMass(const CDCDedxLikelihood* cdcdedx,
                                                   const VXDDedxLikelihood* vxddedx)
  {
    if (!cdcdedx and !vxddedx) return 0;

    std::vector<double> logL;
    std::vector<double> masses;
    std::vector<double> priors;
    for (const auto& type : Const::chargedStableSet) {
      if (cdcdedx and vxddedx) {
        logL.push_back(cdcdedx->getLogL(type) + vxddedx->getLogL(type));
      } else if (cdcdedx) {
        logL.push_back(cdcdedx->getLogL(type));
      } else {
        logL.push_back(vxddedx->getLogL(type));
      }
      masses.push_back(type.getMass());
      priors.push_back(m_priors[abs(type.getPDGCode())]);
    }

    // get maximum
    auto logL_max = logL[0];
    for (auto x : logL) {
      if (x > logL_max) logL_max = x;
    }
    // calculate probabilities, normalizaton is not needed
    std::vector<double> probability(logL.size());
    for (unsigned i = 0; i < logL.size(); ++i) {
      probability[i] = exp(logL[i] - logL_max) * priors[i];
    }

    unsigned i0 = 0;
    for (unsigned i = 0; i < probability.size(); ++i) {
      if (probability[i] > probability[i0]) i0 = i;
    }
    return masses[i0];

  }

} // end Belle2 namespace

