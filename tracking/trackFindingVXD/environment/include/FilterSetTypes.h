/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Jakob Lettenbichler                                            *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/
#pragma once

#include <tracking/spacePointCreation/SpacePoint.h>

// #include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h"
// #include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZTemp.h"
// #include "tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h"
// #include "tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h"
// #include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZSquared.h"
// #include "tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"

#include "tracking/trackFindingVXD/ThreeHitFilters/Angle3DSimple.h"
#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"
#include "tracking/trackFindingVXD/FilterTools/ObserverPrintResults.h"
#include "tracking/trackFindingVXD/FilterTools/Observer.h" // empty observer
#include "tracking/trackFindingVXD/FilterTools/Observer3HitPrintResults.h"

#include "tracking/vxdCaTracking/VXDTFHit.h"

namespace Belle2 {


  /// minimal working example used for redesign of VXDTF.
  typedef decltype((0. <= Distance3DSquared<Belle2::SpacePoint>() <= 0.).observe(ObserverPrintResults())) TwoHitFilterSet;


  /// minimal working example used for showcase using old TF.
  typedef decltype((0. <= Distance3DSquared<Belle2::VXDTFHit>() <= 0.).observe(Observer()).enable()) TwoHitFilterShowCase;




}
