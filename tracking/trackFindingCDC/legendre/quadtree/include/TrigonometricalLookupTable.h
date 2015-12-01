/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Lookup table for sine/cosine for the bin used to partition the Theta space of
 * the quad tree.
 */


#pragma once

#include <framework/logging/Logger.h>
#include <framework/utilities/Utils.h>

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrigonometricalLookupTable {

    public:

      TrigonometricalLookupTable();

      /**
       * Initialize lookup table
       */
      void initialize();

      /**
       * Get instance of class
       */
      static TrigonometricalLookupTable& Instance();

      inline float sinTheta(unsigned long bin)
      {
        if (branch_unlikely(not m_lookup_created)) {
          initialize();
        }
        if (branch_unlikely(bin >= m_lookup_theta.size())) {
          return computeSin(bin);
        } else {
          return m_lookup_theta[bin].first;
        }
      }

      inline float cosTheta(unsigned long bin)
      {
        if (branch_unlikely(not m_lookup_created)) initialize();
        if (branch_unlikely(bin >= m_lookup_theta.size())) {
          return computeCos(bin);
        } else {
          return m_lookup_theta[bin].second;
        }
      }

      inline unsigned long getNBinsTheta() const {return m_nbinsTheta;};

    private:

      inline float computeSin(unsigned long bin) const
      {
        const float bin_width = 2.* boost::math::constants::pi<float>()  / m_nbinsTheta;
        return sin(bin * bin_width - boost::math::constants::pi<float>() + bin_width / 2.);
      }

      inline float computeCos(unsigned long bin) const
      {
        const float bin_width = 2.* boost::math::constants::pi<float>()  / m_nbinsTheta;
        return cos(bin * bin_width - boost::math::constants::pi<float>() + bin_width / 2.);
      }

      std::vector<std::pair<float, float>> m_lookup_theta; /**< Lookup array for calculation of sin */
      bool m_lookup_created; /**< Allows to use the same lookup table for sin and cos */
      unsigned long m_nbinsTheta; /**< Number of theta bins */

    };
  }
}
