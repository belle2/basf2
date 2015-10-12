/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Tadeas Bilka                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <calibration/CalibrationAlgorithm.h>
#include <TProfile.h>
#include <TRandom.h>

namespace Belle2 {
  namespace calibration {
    /**
     * Test class implementing calibration algorithm
     */
    class TestCalibrationAlgorithm : public CalibrationAlgorithm {
    public:

      /// Constructor set the prefix to TestCalibration
      TestCalibrationAlgorithm();

      /// Destructor
      virtual ~TestCalibrationAlgorithm() {}

    protected:

      /// Run algo on data
      virtual E_Result calibrate() override;

    private:
      // TODO: Why I had to do this to get rid warnings during dictionary compilation?
      ClassDefOverride(TestCalibrationAlgorithm, 0); /**< Test class implementing calibration algorithm */

    };
  } // calibration namespace
} // namespace Belle2


