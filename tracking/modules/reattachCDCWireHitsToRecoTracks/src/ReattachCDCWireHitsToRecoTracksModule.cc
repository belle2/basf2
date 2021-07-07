/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/modules/reattachCDCWireHitsToRecoTracks/ReattachCDCWireHitsToRecoTracksModule.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoHitInformation.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(ReattachCDCWireHitsToRecoTracks)

ReattachCDCWireHitsToRecoTracksModule::ReattachCDCWireHitsToRecoTracksModule() :
  Module()
{
  setDescription(R"DOC(
Module to loop over low-ADC/TOT CDCWireHits and RecoTracks
and reattach the hits to the tracks if they are closer
than a given distance.)DOC");
  setPropertyFlags(c_ParallelProcessingCertified); // parallel processing

  addParam("CDCWireHitsStoreArrayName", m_CDCWireHitsStoreArrayName,
           "Name of the input CDCWireHit StoreArray", m_CDCWireHitsStoreArrayName);
  addParam("inputRecoTracksStoreArrayName", m_inputRecoTracksStoreArrayName,
           "Name of the input RecoTrack StoreArray", m_inputRecoTracksStoreArrayName);
  addParam("outputRecoTracksStoreArrayName", m_outputRecoTracksStoreArrayName,
           "Name of the output RecoTrack StoreArray", m_outputRecoTracksStoreArrayName);
  addParam("MaximumDistance", m_maximumDistance,
           "Distance (cm) below which (exclusive) a CDC hit can be reattached to a track", m_maximumDistance);
  addParam("MinimumADC", m_minimumADC,
           "ADC above which (inclusive) a CDC hit can be reattached to a track", m_minimumADC);
  addParam("MinimumTOT", m_minimumTOT,
           "TOT above which (inclusive) a CDC hit can be reattached to a track", m_minimumTOT);
  addParam("MaximumAbsD0", m_maximumAbsD0,
           "Only tracks with an absolute value of d0 below (exclusive) this parameter (cm) are considered", m_maximumAbsD0);
  addParam("MaximumAbsZ0", m_maximumAbsZ0,
           "Only tracks with an absolute value of z0 below (exclusive) this parameter (cm) are considered", m_maximumAbsZ0);
}


void ReattachCDCWireHitsToRecoTracksModule::initialize()
{
  m_CDCWireHits.isRequired(m_CDCWireHitsStoreArrayName);
  m_inputRecoTracks.isRequired(m_inputRecoTracksStoreArrayName);
  m_outputRecoTracks.registerInDataStore(m_outputRecoTracksStoreArrayName);
  RecoTrack::registerRequiredRelations(m_outputRecoTracks);
}


void ReattachCDCWireHitsToRecoTracksModule::event()
{
  m_mapToHitsOnTrack.clear();
  m_mapToHitsToAdd.clear();
  findHits();
  addHits();
}

void ReattachCDCWireHitsToRecoTracksModule::findHits()
{

  TrackFitter trackFitter;

  for (RecoTrack& recoTrack : m_inputRecoTracks) {
    // only fit tracks coming from the IP (d0 and z0 from Helix)
    const Vector3D trackPosition(recoTrack.getPositionSeed());
    const Vector3D trackMomentum(recoTrack.getMomentumSeed());
    const CDCTrajectory3D trajectory(trackPosition, recoTrack.getTimeSeed(), trackMomentum, recoTrack.getChargeSeed());
    const CDCTrajectory2D& trajectory2D(trajectory.getTrajectory2D());
    const CDCTrajectorySZ& trajectorySZ(trajectory.getTrajectorySZ());
    const double d0Estimate((trajectory2D.getClosest(Vector2D(0, 0))).norm());
    const double z0Estimate(trajectorySZ.getZ0());
    if (abs(d0Estimate) < m_maximumAbsD0 and abs(z0Estimate) < m_maximumAbsZ0) {
      if (trackFitter.fit(recoTrack)) {
        m_mapToHitsOnTrack[&recoTrack] = recoTrack.getSortedCDCHitList();
      }
    }
  }

  // Loop over the CDC hits and find the closest track (if any) whose distance to the hit is smaller than the threshold.
  // Only the hits with the BadADCOrTOTFlag are considered (these are hits rejected by the TFCDC_WireHitPreparer module).
  for (CDCWireHit& wireHit : *m_CDCWireHits) {
    if ((wireHit.getAutomatonCell().hasBadADCOrTOTFlag()) and
        (wireHit.getHit()->getADCCount() >= m_minimumADC) and
        (wireHit.getHit()->getTOT() >= m_minimumTOT)) {

      double currentMinimalDistance(m_maximumDistance);
      RecoTrack* currentClosestTrack(nullptr);
      ERightLeft currentRlInfo(ERightLeft::c_Unknown);

      for (RecoTrack& recoTrack : m_inputRecoTracks) {
        if (m_mapToHitsOnTrack.find(&recoTrack) == m_mapToHitsOnTrack.end()) { // Track not considered
          continue;
        }

        bool neighborFound(false);
        for (CDCHit* hitOnTrack : m_mapToHitsOnTrack[&recoTrack]) {

          if (neighborFound) {
            continue;
          }
          // To be added, the hit off track needs to be a neighbor of at least one hit on track
          if (wireHit.getWire().isPrimaryNeighborWith(*CDCWire::getInstance(*hitOnTrack))) {
            neighborFound = true;
          } else {
            continue;
          }

          const ReconstructionResults results(reconstruct(wireHit,
                                                          recoTrack,
                                                          recoTrack.getRecoHitInformation(hitOnTrack)));

          if (not results.isValid) {
            continue;
          }

          if (std::abs(results.distanceToTrack) < currentMinimalDistance) {

            currentMinimalDistance = std::abs(results.distanceToTrack);
            currentClosestTrack = &recoTrack;
            currentRlInfo = results.rlInfo;

            B2DEBUG(29, "Background hit close to the track found..." << std::endl
                    << "Layer of the hit on track: " << hitOnTrack->getICLayer() << std::endl
                    << "Layer of the background hit: " << wireHit.getHit()->getICLayer() << std::endl
                    << "ID of the background hit: " << wireHit.getHit()->getID() << std::endl
                    << "ADC of the background hit: " << wireHit.getHit()->getADCCount() << std::endl
                    << "TOT of the background hit: " << wireHit.getHit()->getTOT() << std::endl
                    << "Distance from track to hit: " << results.distanceToTrack << std::endl);
          }
        }
      }

      if (currentMinimalDistance < m_maximumDistance) { // This hit needs to be added to a RecoTrack
        HitToAddInfo hitToAddInfo;
        hitToAddInfo.hit = &wireHit;
        hitToAddInfo.rlInfo = currentRlInfo;
        m_mapToHitsToAdd[currentClosestTrack].emplace_back(hitToAddInfo);
      }
    }
  }
}


void ReattachCDCWireHitsToRecoTracksModule::addHits()
{

  for (RecoTrack& recoTrack : m_inputRecoTracks) {

    RecoTrack* newRecoTrack = recoTrack.copyToStoreArray(m_outputRecoTracks);

    if (m_mapToHitsToAdd.find(&recoTrack) == m_mapToHitsToAdd.end()) { // No hit to add

      newRecoTrack->addHitsFromRecoTrack(&recoTrack);

    } else { // At least one hit to add

      std::unordered_map<CDCWireHit*, double> previousArcLength;
      std::unordered_map<CDCWireHit*, double> currentArcLength;
      // Initialise the arc-length maps to zero and unset the taken and background flags.
      for (HitToAddInfo& hitToAddInfo : m_mapToHitsToAdd[&recoTrack]) {
        previousArcLength[hitToAddInfo.hit] = 0.0;
        currentArcLength[hitToAddInfo.hit] = 0.0;
        (hitToAddInfo.hit)->getAutomatonCell().setTakenFlag(false);
        (hitToAddInfo.hit)->getAutomatonCell().setBackgroundFlag(false);
      }

      unsigned int sortingParameter(0);
      for (CDCHit* hitOnTrack : m_mapToHitsOnTrack[&recoTrack]) {
        for (HitToAddInfo& hitToAddInfo : m_mapToHitsToAdd[&recoTrack]) {
          CDCWireHit& hitToAdd = *(hitToAddInfo.hit);
          if (not hitToAdd.getAutomatonCell().hasTakenFlag()) {

            const ReconstructionResults results(reconstruct(hitToAdd,
                                                            recoTrack,
                                                            recoTrack.getRecoHitInformation(hitOnTrack)));

            previousArcLength[&hitToAdd] = currentArcLength[&hitToAdd];
            currentArcLength[&hitToAdd] = results.arcLength;

            B2DEBUG(29, "Hit to be added..." << std::endl
                    << "Layer of the hit on track: " << hitOnTrack->getICLayer() << std::endl
                    << "Layer of the background hit: " << hitToAdd.getHit()->getICLayer() << std::endl
                    << "ID of the background hit: " << hitToAdd.getHit()->getID() << std::endl
                    << "ADC of the background hit: " << hitToAdd.getHit()->getADCCount() << std::endl
                    << "TOT of the background hit: " << hitToAdd.getHit()->getTOT() << std::endl
                    << "Distance from track to hit: " << results.distanceToTrack << std::endl
                    << "Previous arc lenght of the hit: " << previousArcLength[&hitToAdd] << std::endl
                    << "Current arc lenght of the hit: " << currentArcLength[&hitToAdd] << std::endl);

            if ((previousArcLength[&hitToAdd] > 0) and (currentArcLength[&hitToAdd] < 0)) { // Hit needs to be added here.

              const RecoHitInformation::RightLeftInformation rl = rightLeftInformationTranslator(hitToAddInfo.rlInfo);
              newRecoTrack->addCDCHit(hitToAdd.getHit(), sortingParameter, rl, RecoHitInformation::c_ReattachCDCWireHitsToRecoTracks);
              hitToAdd.getAutomatonCell().setTakenFlag(true);
              ++sortingParameter;

            }
          }
        }
        const RecoHitInformation::RightLeftInformation rl = recoTrack.getRecoHitInformation(hitOnTrack)->getRightLeftInformation();
        const RecoHitInformation::OriginTrackFinder foundBy = recoTrack.getRecoHitInformation(hitOnTrack)->getFoundByTrackFinder();
        newRecoTrack->addCDCHit(hitOnTrack, sortingParameter, rl, foundBy);
        //TODO: In the (rare) case where more than one hit are added between the same 2 hits on track, one should order them w.r.t. the arcLength.
        ++sortingParameter;
      }
    }
  }
}


ReattachCDCWireHitsToRecoTracksModule::ReconstructionResults ReattachCDCWireHitsToRecoTracksModule::reconstruct(
  const CDCWireHit& wireHit,
  const RecoTrack& recoTrack, const RecoHitInformation* const recoHitInformation) const
{
  ReconstructionResults results;

  try {

    const genfit::MeasuredStateOnPlane& mSoP(recoTrack.getMeasuredStateOnPlaneFromRecoHit(recoHitInformation));
    const Vector3D trackPosition(mSoP.getPos());
    const Vector3D trackMomentum(mSoP.getMom());
    const CDCTrajectory3D trajectory(trackPosition, mSoP.getTime(), trackMomentum, mSoP.getCharge());

    const CDCTrajectory2D& trajectory2D(trajectory.getTrajectory2D());
    const CDCTrajectorySZ& trajectorySZ(trajectory.getTrajectorySZ());

    Vector2D recoPos2D;
    if (wireHit.isAxial()) {
      recoPos2D = wireHit.reconstruct2D(trajectory2D);
    } else {
      const CDCWire& wire(wireHit.getWire());
      const Vector2D& posOnXYPlane(wireHit.reconstruct2D(trajectory2D));

      const double arcLength(trajectory2D.calcArcLength2D(posOnXYPlane));
      const double z(trajectorySZ.mapSToZ(arcLength));

      const Vector2D& wirePos2DAtZ(wire.getWirePos2DAtZ(z));

      const Vector2D& recoPosOnTrajectory(trajectory2D.getClosest(wirePos2DAtZ));
      const double driftLength(wireHit.getRefDriftLength());
      Vector2D disp2D(recoPosOnTrajectory - wirePos2DAtZ);
      disp2D.normalizeTo(driftLength);
      recoPos2D = wirePos2DAtZ + disp2D;
    }


    results.arcLength = trajectory2D.calcArcLength2D(recoPos2D);
    results.z =  trajectorySZ.mapSToZ(results.arcLength);
    results.distanceToTrack = trajectory2D.getDist2D(recoPos2D);

    const Vector3D hitPosition(wireHit.getWire().getWirePos3DAtZ(trackPosition.z()));

    Vector3D trackPosToWire{hitPosition - trackPosition};
    results.rlInfo =  trackPosToWire.xy().isRightOrLeftOf(trackMomentum.xy());

    results.isValid = true;

  } catch (...) {
    B2WARNING("Distance measurement does not work.");
    results.isValid = false;
  }

  return results;
}


RecoHitInformation::RightLeftInformation ReattachCDCWireHitsToRecoTracksModule::rightLeftInformationTranslator(
  ERightLeft rlInfo) const
{
  using RightLeftInformation = RecoHitInformation::RightLeftInformation;
  if (rlInfo == ERightLeft::c_Left) {
    return RightLeftInformation::c_left;
  } else if (rlInfo == ERightLeft::c_Right) {
    return RightLeftInformation::c_right;
  } else if (rlInfo == ERightLeft::c_Invalid) {
    return RightLeftInformation::c_invalidRightLeftInformation;
  } else {
    return RightLeftInformation::c_undefinedRightLeftInformation;
  }
}
