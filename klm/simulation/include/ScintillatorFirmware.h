/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>

/**
 * @file
 * This file will contain an exact reimplementation of FPGA fitter.
 * Now it is just a fast fitter not using Minuit.
 */

namespace Belle2 {

  namespace KLM {

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
      enum ScintillatorFirmwareFitStatus fit(int* amp, int threshold, KLMScintillatorFirmwareFitResult* fitData);

    private:

      /** Number of points. */
      int m_nPoints;

    };

  }

}
