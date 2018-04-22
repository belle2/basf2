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

#include <eklm/dataobjects/EKLMFPGAFit.h>

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
    class FPGAFitter {

    public:

      /**
       * Constructor.
       * @param[in]  nPoints Number of points in amplitude arrays.
       */
      FPGAFitter(int nPoints);

      /**
       * Destructor.
       */
      ~FPGAFitter();

      /**
       * FPGA fitter.
       * @param[in]  amp       Digital amplitude.
       * @param[in]  threshold Threshold.
       * @param[out] fitData   Fit data.
       * @return Fit status.
       */
      enum FPGAFitStatus fit(int* amp, int threshold, EKLMFPGAFit* fitData);

    private:

      /** Number of points. */
      int m_nPoints;

    };

  }

}

#endif

