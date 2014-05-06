/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Stefan Ferstl, Thomas Fabian        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /** Exception which are thrown by members of the FilterClasses. */
  class FilterExceptions {
  public:
//     friend class ThreeHitFilters;

    BELLE2_DEFINE_EXCEPTION(Straight_Line, "The hits are on a straight Line (or indistinguishably near to being on it).");

    BELLE2_DEFINE_EXCEPTION(Calculating_Curvature_Failed, "It was not possible to determine the curvature of the track (number of times for left handed and right handed curvature was identical)");

    BELLE2_DEFINE_EXCEPTION(Straight_Up, "The line is straight up, so the parametrization y=kx+d makes no sense.");

    BELLE2_DEFINE_EXCEPTION(Duplicate_hit, "Two hits are identical");

    BELLE2_DEFINE_EXCEPTION(Invalid_result_Nan, "Result includes 'nan'-entries!");

    BELLE2_DEFINE_EXCEPTION(Center_Is_Origin, "The Calculated Circle Center is the origin, not supported by this fitter.");

    BELLE2_DEFINE_EXCEPTION(Circle_too_small, "The radius of the circle is too small for usefull pT-estimation.");

  };

}
