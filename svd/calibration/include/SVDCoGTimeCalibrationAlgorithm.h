/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luigi Corona, Giulia Casarosa                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>

namespace Belle2 {
  /**
   * Class implementing SVDCoGTimeCalibration calibration algorithm
   */
  class SVDCoGTimeCalibrationAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to SVDCoGTimeCalibrationCollector
    SVDCoGTimeCalibrationAlgorithm(std::string str);

    /// Destructor
    virtual ~SVDCoGTimeCalibrationAlgorithm() {}

    void setMinEntries(int minEntries) {m_minEntries = minEntries;}

    int getMinEntries() {return m_minEntries;}

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:

    std::string m_id;
    int m_ent = 0;
    float m_minEntries = 10000;
  };
} // namespace Belle2


