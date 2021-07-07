/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>

#define DISTANCEINTIME_U_NAME DistanceInTimeUside

namespace Belle2 {

  /** This variable returns the time difference among the U side clusters of the two space points */
  template <typename PointType>
  class DISTANCEINTIME_U_NAME : public SelectionVariable< PointType , 2, double > {
  public:

    /** is expanded as "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DISTANCEINTIME_U_NAME);

    /** return the time difference (ns) among the U side clusters of the two space points */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      return
        outerHit.TimeU() - innerHit.TimeU();
    }
  };

}
