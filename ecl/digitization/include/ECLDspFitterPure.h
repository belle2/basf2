#ifndef ECLDSPFITTERPURE_H
#define ECLDSPFITTERPURE_H

#include <ecl/digitization/EclConfigurationPure.h>

namespace Belle2 {
  namespace ECL {
    double func(int i, int ifine,
                EclConfigurationPure::signalsamplepure_t& signal);
    double func1(int i, int ifine,
                 EclConfigurationPure::signalsamplepure_t& signal);
    void initParams(EclConfigurationPure::fitparamspure_t& params,
                    EclConfigurationPure::signalsamplepure_t& signal);
    void DSPFitterPure(const EclConfigurationPure::fitparamspure_t& params,
                       const int* FitA, const int ttrig,
                       int& amp, double& time, double& chi2, int& niter);
  }
}

#endif
