/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/states/CKFStateObject.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /// Calculate the layer this state is located on.
  inline unsigned int extractGeometryLayer(const CKFStateObject<RecoTrack, SpacePoint>& stateObject)
  {
    return static_cast<unsigned int>((static_cast<double>(stateObject.getNumber()) / 2) + 1);
  }

  /// Calculate the layer this state is located on.
  inline unsigned int extractGeometryLayer(const CKFStateObject<RecoTrack, TrackFindingCDC::CDCRLWireHit>& stateObject)
  {
    return 56 - static_cast<unsigned int>((static_cast<double>(stateObject.getNumber()) / 2) + 1);
  }

  /// Check if this state should describe an overlap hit.
  inline bool isOnOverlapLayer(const CKFStateObject<RecoTrack, SpacePoint>& stateObject)
  {
    return stateObject.getNumber() % 2 == 0;
  }
}