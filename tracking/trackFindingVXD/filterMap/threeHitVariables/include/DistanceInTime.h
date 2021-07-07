/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/filterMap/filterFramework/SelectionVariable.h>

#define DISTANCEINTIME DistanceInTime

namespace Belle2 {

  /** This variable returns the difference among the V and U side clusters of th ecenter space point*/
  template <typename PointType>
  class DISTANCEINTIME : public SelectionVariable< PointType , 3, double > {
  public:

    /** is expanded as "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DISTANCEINTIME);

    /** return the time difference (ns) among the V and U side clusters of th ecenter space point */
    static double value(const PointType&, const PointType& centerHit, const PointType&)

    {
      return
        centerHit.TimeV() - centerHit.TimeU();
    }
  };

}
