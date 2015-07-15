/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/hough/WeightedFastHough.h>
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngle.h>

#include <tracking/trackFindingCDC/hough/QuadChildrenStructure.h>
#include <tracking/trackFindingCDC/hough/OctoChildrenStructure.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    // template<class Item>
    // using Phi0OmegaFastHough = WeightedFastHough<Item,
    //       Box<DiscreteAngle, float>,
    //       QuadChildrenStructure>;

    // template<class Item>
    // using D0Phi0FastHough = WeightedFastHough<Item,
    //       Box<float, DiscreteAngle>,
    //       QuadChildrenStructure>;

    // template<class Item>
    // using D0Phi0OmegaFastHough = WeightedFastHough<Item,
    //       Box<float, DiscreteAngle, float>,
    //       OctoChildrenStructure>;

    // template<class Item>
    // using Phi0OmegaTanLambdaFastHough = WeightedFastHough<Item,
    //       Box<DiscreteAngle, float, float>,
    //       OctoChildrenStructure>;

  }
}
