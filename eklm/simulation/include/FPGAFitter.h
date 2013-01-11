/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FPGAFITTER_H
#define FPGAFITTER_H

/**
 * @file
 * This file will contain an exact reimplementation of FPGA fitter.
 * Now it is just a fast fitter not using Minuit.
 */

namespace Belle2 {

  namespace EKLM {

    /** 2 ** (ADC resolution in bits) */
    const int ADCRange = 4096;          /* TODO: replace by real value! */

    /**
     * FPGA fit status.
     */
    enum FPGAFitStatus {
      c_FPGASuccessfulFit, /**< Successful fit. */
      c_FPGANoSignal,      /**< Signal is too small to do any fitting. */
    };

    /**
     * Fit parameters/results.
     */
    struct FPGAFitParams {
      double startTime;       /**< Start of signal. */
      double peakTime;        /**< Peak time (from startTime). */
      double attenuationFreq; /**< Attenuation frequency. */
      double amplitude;       /**< Amplitude. */
      double bgAmplitude;     /**< Background amplitude. */
    };

    /**
     * FPGA fitter.
     * @param[in]  amp     Digital amplitude.
     * @param[in]  fit     Fit result histogram.
     * @param[in]  nPoints Number of points in amplitude array.
     * @param[out] par     Fit parameters.
     * @return Fit status.
     */
    enum FPGAFitStatus FPGAFit(int* amp, float* fit, int nPoints,
                               struct FPGAFitParams* par);

  }

}

#endif

