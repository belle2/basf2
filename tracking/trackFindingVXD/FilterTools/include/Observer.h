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



// #include <tuple>
// #include <string>
// #include <iostream>
// #include "tracking/vxdCaTracking/VXDTFHit.h"
// #include "tracking/trackFindingVXD/FilterTools/Range.h"
// #include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"
// #include "tracking/vxdCaTracking/VXDTFHit.h"
// #include <iostream>


namespace Belle2 {

  class Observer {
  public:

    /// version Eugenio:
//     template< class T, class someFloat>
    //    static void notify( Belle2::Distance3DSquared<Belle2::VXDTFHit> , double val, const Belle2::VXDTFHit &,
//     static void notify(T , double val,  Belle2::Range< someFloat, someFloat> m_range,
//                        const Belle2::VXDTFHit&, const Belle2::VXDTFHit&)
    /// version Jakob 1:
//  template< class T, class someRangeType>
//  static void notify(T , double val,  someRangeType m_range,
    //             const VXDTFHit&, const VXDTFHit&)
    /// version Jakob 2:
    template< class T, class someRangeType, class someHitType>
    static void notify(T , double val,  someRangeType m_range,
                       const someHitType&, const someHitType&)
    {
      //std::cout << m_range.getInf() << " < " << val  << " < " << m_range.getSup() << " | ";
    };

    Observer() {};
  };
}
