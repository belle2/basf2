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

    /** this exception is thrown by the CircleFit and occurs when the track is too straight */
    BELLE2_DEFINE_EXCEPTION(Straight_Line, "The hits are on a straight Line (or indistinguishably near to being on it).");

    /** this exception is thrown by the TrackletFilters::CalcCurvature, when the track is ziggzagging all the time */
    BELLE2_DEFINE_EXCEPTION(Calculating_Curvature_Failed, "It was not possible to determine the curvature of the track (number of times for left handed and right handed curvature was identical)");

    /** this exception is thrown by the StraightLineFit, which currently has a weakness for tracks pointing parallel to the y-axis */
    BELLE2_DEFINE_EXCEPTION(Straight_Up, "The line is straight up, so the parametrization y=kx+d makes no sense.");

    /** this exception is currently not used and should indicate, if the fitter was filled with the same hit more than once in one tc */
    BELLE2_DEFINE_EXCEPTION(Duplicate_hit, "Two hits are identical");

    /** this exception is thrown by the momentumSeed-calculator and the helixFit, if the input was not fittable */
    BELLE2_DEFINE_EXCEPTION(Invalid_result_Nan, "Result includes 'nan'-entries!");

    /** The circle- and the helixFit can not fit tracks whose projected circle center is in the origin. Since this should never happen for real tracks, such tracks can easily be neglected */
    BELLE2_DEFINE_EXCEPTION(Center_Is_Origin, "The Calculated Circle Center is the origin, not supported by this fitter.");

    /** this exception catches TCs which are too small to be able to be detected by the TC, therefore are likely to be ghost TCs. such TCs can be neglected */
    BELLE2_DEFINE_EXCEPTION(Circle_too_small, "The radius of the circle is too small for usefull pT-estimation.");

  };

}
