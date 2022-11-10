/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dbobjects/PIDCalibrationWeight.h>

namespace Belle2 {

  /**
   * Module that creates WeightMatrix and uploads it to the DB
   */
  class PIDCalibrationWeightCreatorModule : public Module {

    /// 6x6 weight matrix.
    WeightMatrix m_weightMatrix;

    std::string m_matrixName; /**< Name of the matrix  */

    int m_experimentLow; /**< Lowest valid experiment # */
    int m_experimentHigh; /**< Highest valid experiment # */
    int m_runLow; /**< Lowest valid run # */
    int m_runHigh; /**< Highest valid run # */

  public:

    /**
     * Constructor
     */
    PIDCalibrationWeightCreatorModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

  };

} // Belle2 namespace
