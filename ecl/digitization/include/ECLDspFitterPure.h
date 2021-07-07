/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
