/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni,                                               *
 *               Jakob Lettenbichler                                            *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/
#ifndef VXDTFFILTERS_HH
#define VXDTFFILTERS_HH

#include "tracking/dataobjects/FullSecID.h"

#include <tracking/spacePointCreation/SpacePoint.h>

#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZTemp.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h"
#include "tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"

#include "tracking/trackFindingVXD/ThreeHitFilters/Angle3DSimple.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/AngleXYSimple.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/AngleRZSimple.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/CircleDist2IP.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/DeltaSlopeRZ.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/DeltaSlopeZoverS.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/DeltaSoverZ.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/HelixParameterFit.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/Pt.h"
#include "tracking/trackFindingVXD/ThreeHitFilters/CircleRadius.h"

#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"
#include "tracking/trackFindingVXD/FilterTools/ObserverPrintResults.h"
#include "tracking/trackFindingVXD/FilterTools/Observer.h" // empty observer
#include "tracking/trackFindingVXD/FilterTools/VoidObserver.h" // empty observer

#include "tracking/trackFindingVXD/FilterTools/Observer3HitPrintResults.h"

#include <tracking/dataobjects/SectorMapConfig.h>

#include "vxd/dataobjects/VxdID.h"
#include "tracking/trackFindingVXD/sectorMapTools/CompactSecIDs.h"
#include "tracking/trackFindingVXD/segmentNetwork/StaticSector.h"

//#include <unordered_map>
#include <set>

#include <functional>

namespace Belle2 {


  template<class point_t>
  class VXDTFFilters {
  public:


    /// minimal working 2-hits-example used for redesign of VXDTF.
//     typedef decltype((0. <= Distance3DSquared<Belle2::SpacePoint>() <= 0.).observe(ObserverPrintResults())) twoHitFilter_t;


    /// big working 2-hits-example used for redesign of VXDTF.
    typedef decltype(
      (0. <= Distance3DSquared<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
      (0. <= Distance2DXYSquared<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
      (0. <= Distance1DZ<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
      (0. <= SlopeRZ<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
      (0. <= Distance3DNormed<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())
    ) twoHitFilter_t;


    /// minimal working example for 3-hits:
//  typedef decltype((0. <= Angle3DSimple<point_t>()   <= 0.).observe(Observer3HitPrintResults())) threeHitFilter_t;


    /// big working example for 3-hits:
    typedef decltype(
      (0. <= Angle3DSimple<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= AngleXYSimple<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= AngleRZSimple<point_t>()   <= 0.).observe(VoidObserver())&&
      (CircleDist2IP<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= DeltaSlopeRZ<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= DeltaSlopeZoverS<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= DeltaSoverZ<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= HelixParameterFit<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= Pt<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= CircleRadius<point_t>()   <= 0.).observe(VoidObserver())
    ) threeHitFilter_t;


    /// typedef to make a static sector type more readable.
    typedef StaticSector< point_t, twoHitFilter_t, threeHitFilter_t , int > staticSector_t;


    VXDTFFilters(): m_testConfig() {m_staticSectors.resize(2);}


    int addSectorsOnSensor(const vector<double>&                normalizedUsup,
                           const vector<double>&                normalizedVsup,
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


    int addTwoHitFilter(FullSecID outer,
                        FullSecID inner,
                        const twoHitFilter_t& filter)
    {
      // TODO add the friendship relation to the static sector
      if (m_staticSectors.size() < m_compactSecIDsMap[ outer ] or m_compactSecIDsMap[ outer ] == 0)
        return 0;

      m_staticSectors[m_compactSecIDsMap[outer]]->assign2spFilter(inner, filter);
      return 1;
    }



    int addThreeHitFilter(FullSecID outer,
                          FullSecID center,
                          FullSecID inner,
                          const threeHitFilter_t& filter)
    {
      // TODO add the friendship relation to the static sector
      if (m_staticSectors.size() < m_compactSecIDsMap[ outer ] or m_compactSecIDsMap[ outer ] == 0)
        return 0;

      m_staticSectors[m_compactSecIDsMap[outer]]->assign3spFilter(center, inner, filter);
      return 1;
    }


    /// returns compactSecID for given FullSecID, == 0 if not found.
    CompactSecIDs::sectorID_t getCompactID(FullSecID outer) const
    { return m_compactSecIDsMap[outer]; }


    //    const StaticSectorType& getSector(VxdID aSensorID,
    //  std::pair<float, float> normalizedLocalCoordinates) const
    //  {  }

    /// returns pointer to static sector for given fullSecID. if fullSecID is not found, a nullptr is returned.
    const staticSector_t* getStaticSector(const FullSecID secID) const
    {
      auto sectorPosition = m_compactSecIDsMap[ secID ];
      if (sectorPosition == 0) return nullptr;
      return m_staticSectors[ sectorPosition ];
    }


    const twoHitFilter_t getTwoHitFilters(const FullSecID& outer,
                                          const FullSecID& inner) const
    {
      // TODO: sanity checks
      static twoHitFilter_t just_in_case;
      const auto staticSector = m_staticSectors[ m_compactSecIDsMap[ outer ] ];
      // catch case when sector is not part of the sectorMap:
      if (staticSector == nullptr) return just_in_case;
      const auto filter = staticSector->getFilter2sp(inner);
      return filter;
//       return just_in_case;
    }


    /// check if using getFullID() would be safe (true if it is safe):
    bool areCoordinatesValid(VxdID aSensorID, double normalizedU, double normalizedV) const
    {
      return m_compactSecIDsMap.areCoordinatesValid(aSensorID, normalizedU, normalizedV);
    }


    /// returns fullSecID for given sensorID and local coordinates. JKL: what happens here if no FullSecID could be found?
    FullSecID getFullID(VxdID aSensorID, double normalizedU, double normalizedV) const
    {
      // TODO WARNING how to catch bad cases?
      return m_compactSecIDsMap.getFullSecID(aSensorID, normalizedU, normalizedV);
    }


    /// returns the configuration settings for this VXDTFFilters.
    const SectorMapConfig& getConfig(void) const { return m_testConfig; }
    void setConfig(const SectorMapConfig& config) { m_testConfig = config; }


    /// JKL: intended for some checks only - returns CompactIDsMap storing the static sectors.
    const CompactSecIDs& getCompactIDsMap() const { return m_compactSecIDsMap; }


    /// JKL: intended for some checks only - returns CompactIDsMap storing the static sectors.
    const std::vector< staticSector_t*>& getStaticSectors() const { return m_staticSectors; }

    /// returns number of compact secIDs stored for this filter-container.
    unsigned size() const { return m_compactSecIDsMap.getSize(); }
  private:

    vector< staticSector_t* > m_staticSectors;

    CompactSecIDs m_compactSecIDsMap;

    /** configuration  */
    SectorMapConfig m_testConfig;

  };

}


#endif
