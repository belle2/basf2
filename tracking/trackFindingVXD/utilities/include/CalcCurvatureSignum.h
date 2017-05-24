/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/geometry/B2Vector3.h>


namespace Belle2 {
  /** Calculate curvature based on triplets of measurements.
   *  Ignores uncertainties.
   *  Returns -1,0,1 depending on the sum of all triplets.
   */
  inline short calcCurvatureSignum(std::vector<SpacePoint const*> const& measurements)
  {
    if (measurements.size() < 3) return 0;
    float sumOfCurvature = 0.;
    for (unsigned int i = 0; i < measurements.size() - 2; ++i) {
      B2Vector3<double> ab = measurements.at(i + 1)->getPosition() - measurements.at(i)->getPosition();
      ab.SetZ(0.);
      B2Vector3<double> bc = measurements.at(i + 2)->getPosition() - measurements.at(i + 1)->getPosition();
      bc.SetZ(0.);
      sumOfCurvature += bc.Orthogonal() * ab; //normal vector of m_vecBC times segment of ba
    }
    // signum
    return (0 < sumOfCurvature) - (sumOfCurvature < 0);
  }
}
