/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/CDCHit.h>

namespace Belle2 {
  namespace TrackFinderCDCLegendre {

    class ConformalPosition {
    public:

      /**
       * Constructor
       */
      ConformalPosition();

      ~ConformalPosition();

      /**
       * Delete static instance
       */
      void clearPointers();

      /**
       * Get instance of class
       */
      static ConformalPosition& Instance();

      /**
       * Get pre-created instance of class; should be used when static instance 100% has been created
       */
      inline static ConformalPosition& InstanceTrusted() {  return *s_cdcLegendreConformalPosition; };

      /**
       * Ger conformal position of wire
       */
      inline double getConformalR(int layerId, int wireId, int binTheta) const { return m_lookupR[layerId][wireId][binTheta]; };

      /**
       * Get number of wires in layer
       */
      double getNWires(int layerId);


    private:

      double* m_sin_theta; /**< Lookup array for calculation of sin */
      double* m_cos_theta; /**< Lookup array for calculation of cos */
      static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

      static const int nLayers_max = 56; /**< Number of layers */
      static const int nWires_max = 384; /**< max. number of wires in layer */
      static const int m_nbinsTheta = 8192; /**< Number of theta bins (in legendre space) */

      double m_lookupR[nLayers_max][nWires_max][(m_nbinsTheta + 1)]; /**< Lookup table for holding information about r value, used in FastHogh algorithm. */

      static ConformalPosition* s_cdcLegendreConformalPosition; /**< Static instance of the class */

    };
  }
}
