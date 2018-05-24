/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include "TF1.h"
#include "TH2F.h"

namespace Belle2 {
  /**
   * A calibration algorithm for CDC dE/dx electron cos(theta) dependence
   *
   */
  class CDCDedx2DCellAlgorithm : public CalibrationAlgorithm {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the algorithm.
     */
    CDCDedx2DCellAlgorithm();

    /**
     * Destructor
     */
    virtual ~CDCDedx2DCellAlgorithm() {}

  protected:

    /**
     * Run algorithm
     */
    virtual EResult calibrate();

  private:

    /** Save arithmetic and truncated mean for the 'dedx' values.
     *
     * @param dedx              input values
     * @param removeLowest      lowest fraction of hits to remove (0.05)
     * @param removeHighest     highest fraction of hits to remove (0.25)
     */
    double calculateMean(const std::vector<double>& dedx, double removeLowest, double removeHighest) const;

  };
} // namespace Belle2
