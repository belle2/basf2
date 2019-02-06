/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>

/**
 * @file
 * This file will contain an exact reimplementation of FPGA fitter.
 * Now it is just a fast fitter not using Minuit.
 */

namespace Belle2 {

  namespace EKLM {

    /**
     * FPGA fitter class.
     */
    class ScintillatorFirmware {

    public:

      /**
       * Constructor.
       * @param[in]  nPoints Number of points in amplitude arrays.
       */
      explicit ScintillatorFirmware(int nPoints);

      /**
       * Destructor.
       */
      ~ScintillatorFirmware();

      /**
       * FPGA fitter.
       * @param[in]  amp       Digital amplitude.
       * @param[in]  threshold Threshold.
       * @param[out] fitData   Fit data.
       * @return Fit status.
       */
      enum FPGAFitStatus fit(int* amp, int threshold, KLMScintillatorFirmwareFitResult* fitData);

    private:

      /** Number of points. */
      int m_nPoints;

    };

  }

}
