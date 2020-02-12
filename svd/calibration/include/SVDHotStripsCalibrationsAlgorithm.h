/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani (2019)                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing SVDHotStripsCalibrations calibration algorithm
   */
  class SVDHotStripsCalibrationsAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to SVDHotStripsCalibrationsCollector
    explicit SVDHotStripsCalibrationsAlgorithm(const std::string& str);

    /// Destructor
    virtual ~SVDHotStripsCalibrationsAlgorithm() {}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    std::string m_id; /**< Parameter given to set the UniqueID of the payload*/
    bool theHSFinder(double* stripOccAfterAbsCut, int* hsflag, int nstrips); /**< returns true if the strip is hot*/
    int m_base; /**< parameter to tune finder angorithm*/
    float m_relOccPrec; /**< parameter to tue the finder algorithm*/
  };
} // namespace Belle2


