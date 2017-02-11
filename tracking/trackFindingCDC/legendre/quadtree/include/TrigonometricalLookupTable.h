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
#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class which holds lookup table for sin and cos functions.
     * Deepness (grid level) of the table could be modified via template parameter. Number of bins in the table is 2^level.
     * Since default value for quadtree search is 12 as default deepness of 16 has been chosen to keep ability to perform bins overlapping.
     */
    template<unsigned int maxLvl = PrecisionUtil::getLookupGridLevel()>
    class TrigonometricalLookupTable {

    public:

      /// Constructor
      TrigonometricalLookupTable()
        : m_nbinsTheta(pow(2, maxLvl))
      {
        m_lookup_theta.resize(m_nbinsTheta + 1);
        for (unsigned int i = 0; i <= m_nbinsTheta; ++i) {
          m_lookup_theta[i] = Vector2D(computeCos(i), computeSin(i));
        }
      };

      /// Get instance of the lookup table
      static TrigonometricalLookupTable& Instance()
      {
        static TrigonometricalLookupTable trigonometricalLookupTable;
        return trigonometricalLookupTable;
      }

      /// Get unit direction corresponding to the given bin
      Vector2D thetaVec(unsigned int bin) const
      {
        if (branch_unlikely(bin >= m_lookup_theta.size())) {
          return Vector2D(computeCos(bin), computeSin(bin));
        }
        return m_lookup_theta.at(bin);
      }

      /// Get sin() corresponding to the given bin
      double sinTheta(unsigned int bin) const
      {
        if (branch_unlikely(bin >= m_lookup_theta.size())) {
          return computeSin(bin);
        }
        return m_lookup_theta[bin].y();
      }

      /// Get cos() corresponding to the given bin
      double cosTheta(unsigned int bin) const
      {
        if (branch_unlikely(bin >= m_lookup_theta.size())) {
          return computeCos(bin);
        }
        return m_lookup_theta[bin].x();
      }

      /// Get number of bins in the lookup table
      unsigned int getNBinsTheta() const
      {
        return m_nbinsTheta;
      };

    private:

      /// Compute cos for the given bin value
      double computeSin(unsigned int bin) const
      {
        const float bin_width = 2.* boost::math::constants::pi<float>() / m_nbinsTheta;
        return sin(bin * bin_width - boost::math::constants::pi<float>() + bin_width / 2.);
      }

      /// Compute cos for the given bin value
      double computeCos(unsigned int bin) const
      {
        const float bin_width = 2.* boost::math::constants::pi<float>() / m_nbinsTheta;
        return cos(bin * bin_width - boost::math::constants::pi<float>() + bin_width / 2.);
      }

      /// Lookup array for calculation of sin
      std::vector<Vector2D> m_lookup_theta;

      ///  Number of theta bins
      unsigned int m_nbinsTheta;
    };
  }
}
