/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <ecl/digitization/EclConfigurationPure.h>

namespace Belle2 {
  namespace ECL {
    double func(int i, int ifine,
                const EclConfigurationPure::signalsamplepure_t& signal);
    double func1(int i, int ifine,
                 const EclConfigurationPure::signalsamplepure_t& signal);
    void initParams(EclConfigurationPure::fitparamspure_t& params,
                    const EclConfigurationPure::signalsamplepure_t& signal);
    void DSPFitterPure(const EclConfigurationPure::fitparamspure_t& params,
                       const int* FitA, const int ttrig,
                       int& amp, double& time, double& chi2, int& niter);
  }
}
