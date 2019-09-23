/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>
#include <TH1.h>

namespace Belle2 {

  /** Class implementing SVDCrossTalkCalibrations calibration algorithm. */

  class SVDCrossTalkCalibrationsAlgorithm : public CalibrationAlgorithm {
  public:

    /** Constructor  */
    SVDCrossTalkCalibrationsAlgorithm(std::string str);

    /** Destructor */
    virtual ~SVDCrossTalkCalibrationsAlgorithm() {}

  protected:

    virtual EResult calibrate() override;

  private:

    std::string m_id;


  };


} // namespace Belle2
