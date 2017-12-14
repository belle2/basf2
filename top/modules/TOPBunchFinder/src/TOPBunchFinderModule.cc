/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager


// Own include
#include <top/modules/TOPBunchFinder/TOPBunchFinderModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

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
    setDescription("Bunch finder");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("bunchHalfRange", m_bunchHalfRange,
             "Half range of bunch numbers to be searched for", 5);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use full TDC range)", 51.2);
    addParam("sigmaSmear", m_sigmaSmear,
             "sigma in [ns] for additional smearing of PDF", 0.2);
    addParam("minSignal", m_minSignal,
             "minimal number of signal photons to accept track", 10.0);
    addParam("minSBRatio", m_minSBRatio,
             "minimal signal-to-background ratio to accept track", 0.0);
    addParam("maxDERatio", m_maxDERatio,
             "maximal ratio of detected-to-expected photons to accept track", 2.5);
    addParam("useMCTruth", m_useMCTruth,
             "use MC truth for mass instead of that determined from dEdx", false);
  }

  TOPBunchFinderModule::~TOPBunchFinderModule()
  {
  }

  void TOPBunchFinderModule::initialize()
  {
    // input

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<CDCDedxLikelihood> cdcDedxLikelihoods;
    cdcDedxLikelihoods.isRequired();

    StoreArray<VXDDedxLikelihood> vxdDedxLikelihoods;
    vxdDedxLikelihoods.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();

    // output

    StoreObjPtr<TOPRecBunch> recBunch;
    recBunch.registerInDataStore();

    // Configure TOP detector

    TOPconfigure config;

    // bunch separation in time

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_bunchTimeSep = geo->getNominalTDC().getBunchSeparationTime();

    // other

    if (m_maxTime <= 0) m_maxTime = config.getTDCTimeRange();

  }

  void TOPBunchFinderModule::beginRun()
  {
  }

  void TOPBunchFinderModule::event()
  {

    // define output for reconstructed bunch values

    StoreObjPtr<TOPRecBunch> recBunch;
    if (!recBunch.isValid()) recBunch.create();

    // create reconstruction object and set various options

    int Nhyp = 1;
    double mass = Const::pion.getMass();
    TOPreco reco(Nhyp, &mass);
    reco.setPDFoption(TOPreco::c_Rough);
    reco.setTmax(m_maxTime + m_bunchTimeSep * m_bunchHalfRange);

    // add photon hits to reconstruction object

    StoreArray<TOPDigit> topDigits;
    for (const auto& digit : topDigits) {
      if (digit.getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                     digit.getTimeError());
    }

    // create working variables

    std::vector<double> t0;
    std::vector<double> logL;
    int n = 2 * m_bunchHalfRange + 1;
    for (int i = 0; i < n; i++) {
      t0.push_back((i - m_bunchHalfRange) * m_bunchTimeSep);
      logL.push_back(0);
    }
    int numTrk = 0;
    int usedTrk = 0;

    // loop over reconstructed tracks and make sum of log likelihoods for diff. bunches
    StoreArray<Track> tracks;
    for (const auto& track : tracks) {
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // determine most probable particle mass
      if (m_useMCTruth) {
        if (!trk.getMCParticle()) continue;
        if (!trk.getBarHit()) continue;
        mass = trk.getMCParticle()->getMass();
      } else {
        auto cdcdedx = track.getRelated<CDCDedxLikelihood>();
        auto vxddedx = track.getRelated<VXDDedxLikelihood>();
        if (!cdcdedx and !vxddedx) {
          mass = Const::pion.getMass();
          B2WARNING("TOPBunchFinder: no relation to DedxLikelihood - "
                    "pion mass used instead");
        } else {
          mass = getMostProbableMass(cdcdedx, vxddedx);
        }
      }

      // reconstruct (e.g. set PDF internally)
      reco.setMass(mass);
      reco.reconstruct(trk);
      if (reco.getFlag() != 1) continue; // track is not in the acceptance of TOP
      numTrk++;

      // do further track selection
      double expPhot = reco.getExpectedPhotons();
      double expBG = reco.getExpectedBG();
      double expSignal = expPhot - expBG;
      double numPhot = reco.getNumOfPhotons();
      if (expSignal < m_minSignal) continue;
      if (expSignal < m_minSBRatio * expBG) continue;
      if (numPhot > m_maxDERatio * expPhot) continue;
      usedTrk++;

      // make sum of log likelihoods for different bunches
      for (int i = 0; i < n; i++) {
        logL[i] += reco.getLogL(t0[i], m_maxTime, m_sigmaSmear);
      }
    }

    // find maximum

    int i0 = m_bunchHalfRange;
    for (int i = 0; i < n; i++) {
      if (logL[i] > logL[i0]) i0 = i;
    }

    // store the results

    int bunchNo = i0 - m_bunchHalfRange;
    double bunchTime = t0[i0];
    recBunch->setReconstructed(bunchNo, bunchTime, numTrk, usedTrk);
    for (int i = 0; i < n; i++) recBunch->addLogL(logL[i] - logL[i0]);

    // correct time in TOPDigits

    for (auto& digit : topDigits) digit.subtractT0(-bunchTime);

  }


  void TOPBunchFinderModule::endRun()
  {
  }

  void TOPBunchFinderModule::terminate()
  {
  }




  double TOPBunchFinderModule::getMostProbableMass(const CDCDedxLikelihood* cdcdedx, const VXDDedxLikelihood* vxddedx) const
  {
    if (!cdcdedx and !vxddedx) return 0;

    std::vector<double> logL;
    std::vector<double> mass;
    //    for(auto type: Const::chargedStableSet) { --- not implemented yet
    for (auto type = Const::chargedStableSet.begin();
         type != Const::chargedStableSet.end(); ++type) {
      if (cdcdedx and vxddedx) {
        logL.push_back(cdcdedx->getLogL(type) + vxddedx->getLogL(type));
      } else if (cdcdedx) {
        logL.push_back(cdcdedx->getLogL(type));
      } else {
        logL.push_back(vxddedx->getLogL(type));
      }
      mass.push_back(type.getMass());
    }
    unsigned i0 = Const::pion.getIndex();
    for (unsigned i = 0; i < logL.size(); ++i) {
      if (logL[i] > logL[i0]) i0 = i;
    }
    return mass[i0];

  }


} // end Belle2 namespace

