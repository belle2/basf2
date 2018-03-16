/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Malwin Weiler, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/vxdCDCTrackMerger/ExtrapolationDetectorTrackCombinationSelector.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/MeasuredStateOnPlane.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void ExtrapolationDetectorTrackCombinationSelector::exposeParameters(ModuleParamList* moduleParamList,
    const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cutValue"), m_param_cutValue,
                                "The maximal distance of extrapolated tracks defined on a plane with the"
                                " given radius, above the relation will be deleted.",
                                m_param_cutValue);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "radius"), m_param_radius,
                                "Radius to which the two tracks in each relation should be extrapolated. "
                                "This can be for example the CDC inner wall radius.",
                                m_param_radius);
}

void ExtrapolationDetectorTrackCombinationSelector::apply(std::vector<WeightedRelationItem>& weightedRelations)
{
  // This will be the result list
  std::vector<WeightedRelationItem> selectedWeightedRelations;

  // The used track fitting algorithm
  TrackFitter trackFitter;


  // Create a map from all items in the weighted relations (no matter if "from" or "to" items)
  // to their measured state on plane (from first hit for "from" items and from last hit for "to" items).
  // This map will later be used to calculate and extrapolate in a fast manner.
  // Do this only for those items, that are fitable.
  std::map<RecoTrack*, genfit::MeasuredStateOnPlane> itemsWithStates;

  for (const auto& weightedRelation : weightedRelations) {
    RecoTrack* cdcTrack = *(weightedRelation.getFrom());
    RecoTrack* vxdTrack = *(weightedRelation.getTo());

    // TODO: if the extrapolation failed, we do not want to try it again
    // TODO: put both functions somehow together

    if (itemsWithStates.find(cdcTrack) == itemsWithStates.end()) {
      // No matter if the last fit failed or not, a representation is added anyways
      if (cdcTrack->getRepresentations().empty()) {
        trackFitter.fit(*cdcTrack);
      }

      // Skip non-fittable tracks
      if (not cdcTrack->wasFitSuccessful()) {
        continue;
      }

      genfit::MeasuredStateOnPlane stateOnPlane = cdcTrack->getMeasuredStateOnPlaneFromFirstHit();

      try {
        stateOnPlane.extrapolateToCylinder(m_param_radius);
      } catch (genfit::Exception const& exception) {
        // if the extrapolation is not possible, skip this track
        B2DEBUG(9, "CDCTrack extrapolation to cylinder failed: " << exception.what());
        continue;
      }

      itemsWithStates[cdcTrack] = stateOnPlane;
    }

    if (itemsWithStates.find(vxdTrack) == itemsWithStates.end()) {
      // No matter if the last fit failed or not, a representation is added anyways
      if (vxdTrack->getRepresentations().empty()) {
        trackFitter.fit(*vxdTrack);
      }

      // Skip non-fittable tracks
      if (not vxdTrack->wasFitSuccessful()) {
        continue;
      }

      genfit::MeasuredStateOnPlane stateOnPlane = vxdTrack->getMeasuredStateOnPlaneFromLastHit();

      try {
        stateOnPlane.extrapolateToCylinder(m_param_radius);
      } catch (genfit::Exception const& exception) {
        // if the extrapolation is not possible, skip this track
        B2DEBUG(9, "VXDTrack extrapolation to cylinder failed: " << exception.what());
        continue;
      }

      itemsWithStates[vxdTrack] = stateOnPlane;
    }

    // At this state, we always end up with a fittable and extrapolateable combination.
    // Either one of the items was not in the cache, than we tried to fill it in with the code above
    // (which would have called continue if it failed) or it is already in the cache which means, it is fittable.

    // Container for the extrapolated states on planes from before (extrapolated to the CDC wall)
    const genfit::MeasuredStateOnPlane& cdcMeasuredStateOnPlane = itemsWithStates[cdcTrack];
    genfit::MeasuredStateOnPlane vxdMeasuredStateOnPlane = itemsWithStates[vxdTrack];

    // position and momentum of the track extrapolated from the CDC fit to the CDC wall
    const TVector3& cdcPosition = cdcMeasuredStateOnPlane.getPos();
    // position and momentum of the track extrapolated from the VXD fit to the CDC Wall
    TVector3 vxdPosition = vxdMeasuredStateOnPlane.getPos();

    // Check first "rough" distance
    const double distance = sqrt((cdcPosition - vxdPosition) * (cdcPosition - vxdPosition));
    if (distance > m_param_cutValue) {
      continue;
    }

    // After this first check, extrapolate the vxd state to the plane of the cdc state and refine the vxd position.
    try {
      vxdMeasuredStateOnPlane.extrapolateToPlane(cdcMeasuredStateOnPlane.getPlane());
      vxdPosition = vxdMeasuredStateOnPlane.getPos();
    } catch (genfit::Exception const& exception) {
      // if the extrapolation is not possible, skip this track
      B2DEBUG(9, "VXDTrack extrapolation to CDC plane failed: " << exception.what());
      continue;
    }

    // Calculate a weight by using the refined distance
    const double refinedDistance = sqrt((cdcPosition - vxdPosition) * (cdcPosition - vxdPosition));

    selectedWeightedRelations.emplace_back(weightedRelation.getFrom(), 1 / refinedDistance, weightedRelation.getTo());
  }

  // Sort and return the result list
  std::sort(selectedWeightedRelations.begin(), selectedWeightedRelations.end());
  weightedRelations.swap(selectedWeightedRelations);
}
