/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/calibration/TOPPulseHeightAlgorithm.h>

namespace Belle2 {
  namespace TOP {

    TOPPulseHeightAlgorithm::TOPPulseHeightAlgorithm():
      CalibrationAlgorithm("TOPPulseHeightCollector")
    {
      setDescription("Calibration algorithm for pulse-height and threshold efficiency calibration");
    }

    CalibrationAlgorithm::EResult TOPPulseHeightAlgorithm::calibrate()
    {

      return c_OK;
    }


    int TOPPulseHeightAlgorithm::fitPulseHeight(std::shared_ptr<TH1F> h)
    {
      return -1;
    }


  } // end namespace TOP
} // end namespace Belle2
