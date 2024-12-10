/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxPID/CDCDedxHitSaverModule.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/Exception.h>
#include <genfit/MaterialEffects.h>
#include <map>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(CDCDedxHitSaver);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  CDCDedxHitSaverModule::CDCDedxHitSaverModule() : Module()

  {
    // set module description
    setDescription("Module that stores CDC hit information from recoTracks, which is needed for dedx.");
    setPropertyFlags(c_ParallelProcessingCertified);
  }

  CDCDedxHitSaverModule::~CDCDedxHitSaverModule()
  {
  }

  void CDCDedxHitSaverModule::initialize()
  {
    m_tracks.isRequired();
    m_recoTracks.isRequired();
    m_hits.registerInDataStore();
    m_tracks.registerRelationTo(m_hits);

    if (not genfit::MaterialEffects::getInstance()->isInitialized()) {
      B2FATAL("Need to have SetupGenfitExtrapolationModule in path before this one.");
    }

  }

  void CDCDedxHitSaverModule::event()
  {
    // clear output collection
    m_hits.clear();

    // loop over tracks and save CDC hits stored in recoTracks
    for (const auto& track : m_tracks) {
      const RecoTrack* recoTrack = track.getRelatedTo<RecoTrack>();
      if (not recoTrack) {
        B2WARNING("No related recoTrack for this track");
        continue;
      }
      if (recoTrack->getTrackFitStatus()->isTrackPruned()) {
        B2ERROR("GFTrack is pruned, please run CDCDedxHitSaver only on unpruned tracks! Skipping this track.");
        continue;
      }

      // loop over hits of this track
      for (const auto& hitPoint : recoTrack->getHitPointsWithMeasurement()) {
        // get CDCHit
        const auto* rawMeasurement = hitPoint->getRawMeasurement(0);
        if (not rawMeasurement) continue;
        const auto* cdcRecoHit = dynamic_cast<const CDCRecoHit*>(rawMeasurement);
        if (not cdcRecoHit) continue;
        const auto* cdcHit = cdcRecoHit->getCDCHit();
        if (not cdcHit) continue;

        // make sure the fitter info exists
        const auto* fitterInfo = hitPoint->getFitterInfo();
        if (not fitterInfo) continue;

        // check which algorithm found this hit
        const RecoHitInformation* hitInfo = recoTrack->getRecoHitInformation(cdcHit);
        int foundByTrackFinder = hitInfo ? hitInfo->getFoundByTrackFinder() : RecoHitInformation::c_undefinedTrackFinder;

        // get weights
        std::map<int, double> weights;
        for (const auto& rep : recoTrack->getRepresentations()) {
          const auto* kalmanFitterInfo = hitPoint->getKalmanFitterInfo(rep);
          if (not kalmanFitterInfo) continue;
          auto wts = kalmanFitterInfo->getWeights();
          double wt = 0;
          for (double w : wts) if (w > wt) wt = w; // take the largest one (there should be always two, but safer to do in this way)
          weights[abs(rep->getPDG())] = wt;
        }

        // get needed vectors, save the hit and add relation to track
        try {
          const genfit::MeasuredStateOnPlane& mop = fitterInfo->getFittedState();
          auto pocaMom = ROOT::Math::XYZVector(mop.getMom());
          auto pocaOnTrack = ROOT::Math::XYZVector(mop.getPos());
          auto pocaOnWire = ROOT::Math::XYZVector(mop.getPlane()->getO());

          auto* hit = m_hits.appendNew(cdcRecoHit->getWireID(), cdcHit->getTDCCount(), cdcHit->getADCCount(),
                                       pocaMom, pocaOnTrack, pocaOnWire, foundByTrackFinder,
                                       weights[211], weights[321], weights[2212]);
          track.addRelationTo(hit);
        } catch (genfit::Exception&) {
          B2WARNING("Track: " << track.getArrayIndex() << ": genfit::MeasuredStateOnPlane exception occured");
          continue;
        }

      }
    }

  }

} // end Belle2 namespace
