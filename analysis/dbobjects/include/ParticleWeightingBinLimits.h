/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * Just pair of numbers - min and max values of bin border
   */
  class ParticleWeightingBinLimits {
    double m_lowerLimit; /**< lower limit */
    double m_upperLimit; /**< upper limit */
  public:

    /**
     * Constructor
     */
    ParticleWeightingBinLimits() {};

    /**
     * Constructor
     * @param ul some bin border
     * @param ll another bin border
     */
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

    /**
     * Lowest bin border
     * @return Lowest bin border
     */
    double first() const
    {
      return m_lowerLimit;
    }

    /**
     * Highest bin border
     * @return Highest bin border
     */
    double second() const
    {
      return m_upperLimit;
    }
  };
} // Belle2 namespace
