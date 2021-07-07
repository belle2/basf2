/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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


