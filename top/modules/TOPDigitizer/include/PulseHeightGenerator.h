/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include<vector>
#include <math.h>
#include <TRandom.h>

namespace Belle2 {
  namespace TOP {

    /**
     * Generates pulse height according to distribution:
     *        P(x) = (x/x0)^p1 * exp(-(x/x0)^p2), p1 >= 0, p2 > 0
     * in the range 0 to xmax
     */
    class PulseHeightGenerator {

    public:

      /**
       * Default constructor
       */
      PulseHeightGenerator()
      {}

      /**
       * Constructor
       * @param x0 distribution parameter [ADC counts]
       * @param p1 distribution parameter, must be non-negative
       * @param p2 distribution parameter
       * @param xmax upper bound of range in which to generate [ADC counts]
       */
      PulseHeightGenerator(double x0, double p1, double p2, double xmax);

      /**
       * Returns distribution value at x
       * @param x argument [ADC counts]
       * @return value
       */
      double getValue(double x) const
      {
        x /= m_x0;
        return pow(x, m_p1) * exp(-pow(x, m_p2));
      }

      /**
       * Returns generated pulse height
       * @return height [ADC counts]
       */
      double generate() const;

    private:

      double m_x0 = 0; /**< distribution parameter [ADC counts] */
      double m_p1 = 0; /**< distribution parameter, must be non-negative */
      double m_p2 = 0; /**< distribution parameter */
      double m_xmax = 0; /**< upper bound of range [ADC counts] */
      double m_vPeak = 0; /**< peak value */

    };


  } // TOP
} // Belle2
