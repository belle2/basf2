/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <tuple>
#include <iostream>


#include "tracking/vxdCaTracking/VXDTFHit.h"
#include "tracking/trackFindingVXD/FilterTools/Range.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"
#include <iostream>

#include "tracking/vxdCaTracking/VXDTFHit.h"

namespace Belle2 {

  class Observer {
  public:

    template< class T, class someFloat>
    //    static void notify( Belle2::Distance3DSquared<Belle2::VXDTFHit> , double val, const Belle2::VXDTFHit &,
    static void notify(T , double val,  Belle2::Range< someFloat, someFloat> m_range,
                       const Belle2::VXDTFHit&, const Belle2::VXDTFHit&)
    {
      //std::cout << m_range.getInf() << " < " << val  << " < " << m_range.getSup() << " | ";
    };

    Observer() {};
  };
}
