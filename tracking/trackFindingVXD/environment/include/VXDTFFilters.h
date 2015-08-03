/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/
#ifndef VXDTFFILTERS_HH
#define VXDTFFILTERS_HH

#include "tracking/dataobjects/FullSecID.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZTemp.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h"
#include "tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"
#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"
#include "tracking/trackFindingVXD/FilterTools/Observer.h"

#include "tracking/vxdCaTracking/VXDTFHit.h"
#include "tracking/dataobjects/FullSecID.h"

#include "vxd/dataobjects/VxdID.h"
#include "tracking/trackFindingVXD/sectorMapTools/CompactSecIDs.h"
#include "tracking/trackFindingVXD/segmentNetwork/StaticSector.h"

//#include <unordered_map>
#include <set>

#include <functional>

namespace Belle2 {


  class VXDTFFilters {
  public:

    typedef VXDTFHit point_t;

    typedef
    decltype((0.f < Distance3DSquared<point_t>()   < 0.f).observe(Observer()).enable()&&
             (0.f < Distance2DXYSquared<point_t>() < 0.f).observe(Observer()).enable()&&
             (0.f < Distance1DZ<point_t>()         < 0.f)/*.observe(Observer())*/.enable()&&
             (0.f < SlopeRZ<point_t>()             < 0.f).observe(Observer()).enable()&&
             (Distance3DNormed<point_t>() < 0.f).enable()) filter2sp_t;



    typedef StaticSector< point_t, filter2sp_t, int , int > staticSector_t;

    VXDTFFilters() {m_staticSectors.resize(2);}

    int addSectorsOnSensor(const vector<float>&                normalizedUsup,
                           const vector<float>&                normalizedVsup,
                           const vector< vector<FullSecID> >& sectorIds)
    {

      auto addedSectors = m_compactSecIDsMap.addSectors(normalizedUsup,
                                                        normalizedVsup, sectorIds);

      if ((int) addedSectors != ((int) normalizedVsup.size() + 1) *
          ((int) normalizedUsup.size() + 1))
        return addedSectors ;

      addedSectors = 0;
      try {
        for (auto secIDrow : sectorIds)
          for (auto secID : secIDrow) {
            auto compactID = m_compactSecIDsMap.getCompactID(secID);

            if ((int) m_staticSectors.size() <= compactID)
              m_staticSectors.resize(compactID + 1);

            m_staticSectors[ compactID ]  = new staticSector_t(secID) ;
            m_staticSectors[ compactID ]->assignCompactSecIDsMap(m_compactSecIDsMap);

            addedSectors ++;
          }
      } catch (...) { return addedSectors ; }

      return addedSectors;
    }


    int addFriendsSectorFilter(FullSecID inner,
                               FullSecID outer,
                               const filter2sp_t& filter)
    {
      // TODO add the friendship relation to the static sector
      if (m_staticSectors.size() < m_compactSecIDsMap[ outer ])
        return 0;

      m_staticSectors[m_compactSecIDsMap[outer]]->assign2spFilter(inner, filter);
      return 1;

    }


    //    const StaticSectorType& getSector(VxdID aSensorID,
    //  std::pair<float, float> normalizedLocalCoordinates) const
    //  {  }

    const filter2sp_t friendsSectorFilter(const FullSecID& inner,
                                          const FullSecID& outer) const
    {
      // TODO: sanity checks
      static filter2sp_t just_in_case;
      const auto staticSector = m_staticSectors[ m_compactSecIDsMap[ outer ] ];
      const auto filter = staticSector->getFilter2sp(inner);
      return filter;
      return just_in_case;
    }


    FullSecID getFullID(VxdID aSensorID, float x, float y) const
    {
      return m_compactSecIDsMap.getFullID(aSensorID, x, y);
    }

  private:

    vector< staticSector_t* > m_staticSectors;

    CompactSecIDs m_compactSecIDsMap;
  };

}


#endif
