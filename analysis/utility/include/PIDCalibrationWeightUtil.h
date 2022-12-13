/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dbobjects/PIDCalibrationWeight.h>
#include <analysis/dbobjects/PIDDetectorWeights.h>
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
      m_pidCalibWeightDB = std::make_unique<DBObjPtr<PIDCalibrationWeight>>();

      if (!(*m_pidCalibWeightDB))
        B2FATAL("The dbobject PIDCalibrationWeight could not be found! It is necessary for the weighted PID variables.");
    };

    /**
     * Constructor with the name of the calibration weight matrix
     */
    PIDCalibrationWeightUtil(std::string matrixName)
    {
      m_matrixName = matrixName;

      if (m_matrixName.find("PIDDetectorWeights") != std::string::npos) {
        m_pidDetWeightDB = std::make_unique<DBObjPtr<PIDDetectorWeights>>(m_matrixName);
        if (!(*m_pidDetWeightDB))
          B2FATAL("The dbobject PIDCalibrationWeight, " << m_matrixName <<
                  " could not be found! It is necessary for the weighted PID variables.");
      } else {
        m_pidCalibWeightDB = std::make_unique<DBObjPtr<PIDCalibrationWeight>>(m_matrixName);
        if (!(*m_pidCalibWeightDB))
          B2FATAL("The dbobject PIDCalibrationWeight, " << m_matrixName <<
                  " could not be found! It is necessary for the weighted PID variables.");
      }
    };

    /**
     * Get the weight for the given combination of the PDG code and the detector name.
     */
    double getWeight(int pdg, std::string detector, double p, double theta) const
    {
      if (m_matrixName.find("PIDDetectorWeights") != std::string::npos) {

        Const::EDetector det;
        boost::to_lower(detector);
        if (detector == "svd") det = Const::SVD;
        else if (detector == "cdc") det = Const::CDC;
        else if (detector == "top") det = Const::TOP;
        else if (detector == "arich") det = Const::ARICH;
        else if (detector == "ecl") det = Const::ECL;
        else if (detector == "klm") det = Const::KLM;
        else B2FATAL("Unknown detector component: " << detector);

        return (*m_pidDetWeightDB)->getWeight(Const::ParticleType(pdg), det, p, theta);
      } else {
        return (*m_pidCalibWeightDB)->getWeight(pdg, detector);
      }
    }

    /**
     * Get the weight for the given combination of the PDG code and the detector in Const::EDetector.
     */
    double getWeight(int pdg, Const::EDetector det, double p, double theta) const
    {
      if (m_matrixName.find("PIDDetectorWeights") != std::string::npos) {
        return (*m_pidDetWeightDB)->getWeight(Const::ParticleType(pdg), det, p, theta);
      } else {
        return (*m_pidCalibWeightDB)->getWeight(pdg, det);
      }
    }

    /**
     * Get the weights for the given PDG code
     */
    std::vector<double> getWeights(int pdg) const
    {
      return (*m_pidCalibWeightDB)->getWeights(pdg);
    }

  private:
    std::string m_matrixName = "PIDCalibrationWeight"; /**< name of the matrix. */
    std::unique_ptr<DBObjPtr<PIDCalibrationWeight>> m_pidCalibWeightDB; /**< db object for the calibration weight matrix. */
    std::unique_ptr<DBObjPtr<PIDDetectorWeights>>
                                               m_pidDetWeightDB; /**< db object for the detector weight matrix for momentum and theta dependent matrix. */


  };

} // Belle2 namespace




