/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ilya Komarov                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>

namespace Belle2 {

  // Just pair of numners - min and max values of bin border
  //typedef std::pair<double, double> BinLimits;
  class ParticleWeightingBinLimits {
    double m_lowerLimit;
    double m_upperLimit;
  public:
    ParticleWeightingBinLimits() {};
    ParticleWeightingBinLimits(double ul, double ll)
    {
      if (ul > ll) {
        m_upperLimit = ul;
        m_lowerLimit = ll;
      } else if (ul < ll) {
        m_upperLimit = ll;
        m_lowerLimit = ul;

      } else {
        B2FATAL("Trying to create bin with equal limits");
      }
    }
    double first()
    {
      return m_lowerLimit;
    }
    double second()
    {
      return m_upperLimit;
    }
  };
} // Belle2 namespace
