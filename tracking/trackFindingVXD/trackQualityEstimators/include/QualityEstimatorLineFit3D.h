/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler, Felix Metzner, Jonas Wagner         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {

  /** Testbeam:
   * Coords:   Sensors:
   * ^        ./| ./| ./|
   * |   ^    | | | | | |
   * |Y /Z    | | | | | |
   * |/       |/  |/  |/
   * -------> X
   *
   * beam parallel to x. Measurement errors in y & z (v&u)
   * With these conditions, the following approach using 2 independent 2D line fits is acceptable (if rotation is the same for all sensors):
   * Modells:
   * Y_i = a*X_i + b        Z_i = c*X_i + d
   * */
  class QualityEstimatorLineFit3D : public QualityEstimatorBase {

  public:

    virtual double estimateQuality(std::vector<SpacePoint const*> const& measurements) final;

  protected:
  };
}

