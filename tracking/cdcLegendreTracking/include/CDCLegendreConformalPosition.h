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

#include <cdc/dataobjects/CDCHit.h>

namespace Belle2 {

  class CDCLegendreConformalPosition {
  public:

    CDCLegendreConformalPosition();

    ~CDCLegendreConformalPosition();

    void clearPointers();

    static CDCLegendreConformalPosition& Instance();

    inline static CDCLegendreConformalPosition& InstanceTrusted() {  return *s_cdcLegendreConformalPosition; };

    inline double getConformalR(int layerId, int wireId, int binTheta) const { return m_lookupR[layerId][wireId][binTheta]; };

    double getNWires(int layerId);


  private:

    double* m_sin_theta; /**< Lookup array for calculation of sin */
    double* m_cos_theta; /**< Lookup array for calculation of cos */
    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/

    static const int nLayers_max = 56;
    static const int nWires_max = 384;
    static const int m_nbinsTheta = 8192;

    double m_lookupR[nLayers_max][nWires_max][(m_nbinsTheta + 1)]; /**< Lookup table for holding information about r value, used in FastHogh algorithm. */

    static CDCLegendreConformalPosition* s_cdcLegendreConformalPosition;

  };

}
