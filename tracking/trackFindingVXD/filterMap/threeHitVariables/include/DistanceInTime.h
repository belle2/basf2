/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Eugenio Paloni                                           *
*                                                                        *
* This software is provided "as is" without any warranty.                *
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
