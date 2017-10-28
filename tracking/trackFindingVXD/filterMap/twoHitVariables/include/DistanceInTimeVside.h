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

#define DISTANCEINTIME_V_NAME DistanceInTimeVside

namespace Belle2 {

  /** This variable returns the time difference among the V side clusters of the two space points */
  template <typename PointType>
  class DISTANCEINTIME_V_NAME : public SelectionVariable< PointType , 2, double > {
  public:

    /** is expanded as "static const std:string name(void)" frunction which returns name of the Class */
    PUT_NAME_FUNCTION(DISTANCEINTIME_V_NAME);

    /** return the time difference (ns) among the V side clusters of the two space points */
    static double value(const PointType& outerHit, const PointType& innerHit)
    {
      return
        outerHit.TimeV() - innerHit.TimeV();
    }
  };

}
