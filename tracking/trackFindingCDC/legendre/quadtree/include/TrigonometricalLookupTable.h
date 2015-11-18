/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov                                            *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pounsigned longers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include <framework/logging/Logger.h>

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    class TrigonometricalLookupTable {

    public:

      TrigonometricalLookupTable();

      ~TrigonometricalLookupTable();

      /**
       * Delete static instance
       */
      void initialize(bool forced = false);

      /**
       * Delete static instance
       */
      void clearTable();

      /**
       * Get instance of class
       */
      static TrigonometricalLookupTable& Instance();

      inline float sinTheta(unsigned long bin)
      {
        if (not m_lookup_created) initialize();
        return m_sin_theta[bin];
      };

      inline float cosTheta(unsigned long bin)
      {
        if (not m_lookup_created) initialize();
        return m_cos_theta[bin];
      };

      inline unsigned long getNBinsTheta() const {return m_nbinsTheta;};

      void setNBinsTheta(unsigned long nbins) {m_nbinsTheta = nbins;};

    private:

      static constexpr double s_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

      float* m_sin_theta; /**< Lookup array for calculation of sin */
      float* m_cos_theta; /**< Lookup array for calculation of cos */
      bool m_lookup_created; /**< Allows to use the same lookup table for sin and cos */
      unsigned long m_nbinsTheta; /**< Number of theta bins */

    };
  }
}
