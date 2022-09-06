/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dbobjects/PIDCalibrationWeight.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to call calibration weight matrix
   */
  class PIDCalibrationWeightUtil {

  public:

    /**
     * Constructor
     */
    PIDCalibrationWeightUtil()
    {
      m_pidWeightDB = std::make_unique<DBObjPtr<PIDCalibrationWeight>>();

      if (!(*m_pidWeightDB))
        B2FATAL("The dbobject PIDCalibrationWeight could not be found! It is necessary for the weighted PID variables.");
    };

    /**
     * Constructor with the name of the calibration weight matrix
     */
    PIDCalibrationWeightUtil(std::string matrixName)
    {
      m_matrixName = matrixName;
      m_pidWeightDB = std::make_unique<DBObjPtr<PIDCalibrationWeight>>(m_matrixName);

      if (!(*m_pidWeightDB))
        B2FATAL("The dbobject PIDCalibrationWeight, " << m_matrixName <<
                " could not be found! It is necessary for the weighted PID variables.");
    };

    /**
     * Get the weight for the given combination of the PDG code and the detector name.
     */
    double getWeight(int pdg, std::string detector) const
    {
      return (*m_pidWeightDB)->getWeight(pdg, detector);
    }

    /**
     * Get the weight for the given combination of the PDG code and the detector in Const::EDetector.
     */
    double getWeight(int pdg, Const::EDetector det) const
    {
      return (*m_pidWeightDB)->getWeight(pdg, det);
    }

    /**
     * Get the weights for the given PDG code
     */
    std::vector<double> getWeights(int pdg) const
    {
      return (*m_pidWeightDB)->getWeights(pdg);
    }

  private:
    std::string m_matrixName = "PIDCalibrationWeight"; /**< name of the matrix. */
    std::unique_ptr<DBObjPtr<PIDCalibrationWeight>> m_pidWeightDB; /**< db object for the calibration weight matrix. */

  };

} // Belle2 namespace




