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

#include <iostream>
#include "tracking/dataobjects/FullSecID.h"

#include <tracking/spacePointCreation/SpacePoint.h>

#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance1DZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance3DNormed.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/SlopeRZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance1DZSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance3DSquared.h>

#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/Angle3DSimple.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/CosAngleXY.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/AngleRZSimple.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/CircleDist2IP.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/DeltaSlopeRZ.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/DeltaSlopeZoverS.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/DeltaSoverZ.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/HelixParameterFit.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/Pt.h>
#include <tracking/trackFindingVXD/sectorMap/threeHitVariables/CircleRadius.h>

#include <tracking/trackFindingVXD/sectorMap/filterFramework/Shortcuts.h>
#include "tracking/trackFindingVXD/filterTools/ObserverPrintResults.h"
#include <tracking/trackFindingVXD/sectorMap/filterFramework/Observer.h> // empty observer
#include <tracking/trackFindingVXD/sectorMap/filterFramework/VoidObserver.h> // empty observer
#include "tracking/trackFindingVXD/filterTools/ObserverCheckMCPurity.h"

#include "tracking/trackFindingVXD/filterTools/Observer3HitPrintResults.h"

#include <tracking/dataobjects/SectorMapConfig.h>

#include "vxd/dataobjects/VxdID.h"
#include <tracking/trackFindingVXD/sectorMap/map/CompactSecIDs.h>
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
      (
        (0. <= Distance3DSquared<Belle2::SpacePoint>() <= 0.).observe(ObserverCheckMCPurity())&&
        (0. <= Distance2DXYSquared<Belle2::SpacePoint>() <= 0.).observe(ObserverCheckMCPurity())&&
        (0. <= Distance1DZ<Belle2::SpacePoint>() <= 0.).observe(ObserverCheckMCPurity())&&
        (0. <= SlopeRZ<Belle2::SpacePoint>() <= 0.).observe(ObserverCheckMCPurity())&&
        (0. <= Distance3DNormed<Belle2::SpacePoint>() <= 0.).observe(ObserverCheckMCPurity())
      ).observe(ObserverCheckMCPurity())
    ) twoHitFilter_t;

    // March9th2016: TODO: we want to use a big observer observing everything - Working title: MegaObserver.
//  typedef decltype(
//    ((0. <= Distance3DSquared<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
//    (0. <= Distance2DXYSquared<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
//    (0. <= Distance1DZ<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
//    (0. <= SlopeRZ<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())&&
//    (0. <= Distance3DNormed<Belle2::SpacePoint>() <= 0.).observe(VoidObserver())).observe(VoidObserver())
//  ) twoHitFilter_t;

    /// minimal working example for 3-hits:
//  typedef decltype((0. <= Angle3DSimple<point_t>()   <= 0.).observe(Observer3HitPrintResults())) threeHitFilter_t;


    /// big working example for 3-hits:
    typedef decltype(
      (0. <= Angle3DSimple<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= CosAngleXY<point_t>()   <= 0.).observe(VoidObserver())&&
      (0. <= AngleRZSimple<point_t>()   <= 0.).observe(VoidObserver())&&
      (CircleDist2IP<point_t>()         <= 0.).observe(VoidObserver())&&
      (0. <= DeltaSlopeRZ<point_t>()    <= 0.).observe(VoidObserver())&&
      (0. <= DeltaSlopeZoverS<point_t>() <= 0.).observe(VoidObserver())&&
      (0. <= DeltaSoverZ<point_t>()     <= 0.).observe(VoidObserver())&&
      (0. <= HelixParameterFit<point_t>() <= 0.).observe(VoidObserver())&&
      (0. <= Pt<point_t>()              <= 0.).observe(VoidObserver())&&
      (0. <= CircleRadius<point_t>()    <= 0.).observe(VoidObserver())
    ) threeHitFilter_t;


    /// typedef to make a static sector type more readable.
    typedef StaticSector< point_t, twoHitFilter_t, threeHitFilter_t , int >
    staticSector_t;


    VXDTFFilters(): m_testConfig()
    {
      m_staticSectors.resize(2);
      // The first static sector is not used and will never be since the first
      // compact id is 1.
      m_staticSectors[0] = nullptr;
      // initialize the first slot of the Static sector vector
      m_staticSectors[1] = nullptr;
    }


    int addSectorsOnSensor(const std::vector<double>&              normalizedUsup,
                           const std::vector<double>&              normalizedVsup,
                           const std::vector< std::vector<FullSecID> >& sectorIds)
    {

      auto addedSectors = m_compactSecIDsMap.addSectors(normalizedUsup,
                                                        normalizedVsup,
                                                        sectorIds);

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

      if (m_staticSectors.size() <= m_compactSecIDsMap[ outer ] ||
          m_compactSecIDsMap[ outer ] == 0)
        return 0;

      m_staticSectors[m_compactSecIDsMap[outer]]->assign2spFilter(inner, filter);
      return 1;
    }



    int addThreeHitFilter(FullSecID outer,
                          FullSecID center,
                          FullSecID inner,
                          const threeHitFilter_t& filter)
    {

      if (m_staticSectors.size() <= m_compactSecIDsMap[ outer ] ||
          m_compactSecIDsMap[ outer ] == 0 ||
          m_compactSecIDsMap[ center ] == 0 ||
          m_compactSecIDsMap[ inner ] == 0)
        return 0;

      m_staticSectors[m_compactSecIDsMap[outer]]->assign3spFilter(center,
                                                                  inner,
                                                                  filter);
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
      if (staticSector == nullptr)
        return just_in_case;
      const auto filter = staticSector->getFilter2sp(inner);
      return filter;

    }


    /// check if using getFullID() would be safe (true if it is safe):
    bool areCoordinatesValid(VxdID aSensorID,
                             double normalizedU, double normalizedV) const
    {
      return m_compactSecIDsMap.areCoordinatesValid(aSensorID,
                                                    normalizedU, normalizedV);
    }


    /// returns fullSecID for given sensorID and local coordinates.
    // JKL: what happens here if no FullSecID could be found?
    // EP: you get an exception
    FullSecID getFullID(VxdID aSensorID, double normalizedU, double normalizedV) const
    {
      // TODO WARNING how to catch bad cases?
      return m_compactSecIDsMap.getFullSecID(aSensorID, normalizedU, normalizedV);
    }

    /// returns the FullSecId of @param compactSecId
    FullSecID getFullID(CompactSecIDs::sectorID_t compactSecID) const
    {
      return m_staticSectors.at(compactSecID)->getFullSecID();
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

    /// Persists (i.e.: writes) on the current TDirectory the whole object.
    bool persistOnRootFile(void) const
    {

      if (! m_testConfig.Write("config"))
        return false;
      if (! m_compactSecIDsMap.persist())
        return false;

      if (! persistStaticSectors())
        return false;

      return true;
    };

    /// Retrieves from the current TDirectory all the VXDTFFilters
    bool retrieveFromRootFile(void)
    {

      if (! m_testConfig.Read("config"))
        return false;

      if (! m_compactSecIDsMap.read())
        return false;

      if (! retrieveStaticSectors())
        return false;

      return true;
    };

  private:

    /// Persists on the current TDirectory the StaticSectors.
    bool persistStaticSectors(void) const
    {

      TTree* sp2tree = new TTree("SegmentFilters", "SegmentFilters");
      twoHitFilter_t twoHitFilter;
      twoHitFilter.persist(sp2tree, "filter");

      unsigned int outerFullSecID2sp, innerFullSecID2sp;
      sp2tree->Branch("outerFullSecID", & outerFullSecID2sp);
      sp2tree->Branch("innerFullSecID", & innerFullSecID2sp);


      TTree* sp3tree = new TTree("TripletsFilters", "TripletFilters");
      threeHitFilter_t threeHitFilter;
      threeHitFilter.persist(sp3tree, "filter");

      unsigned int outerFullSecID3sp, centerFullSecID3sp,
               innerFullSecID3sp;
      sp3tree->Branch("outerFullSecID", & outerFullSecID3sp);
      sp3tree->Branch("centerFullSecID", & centerFullSecID3sp);
      sp3tree->Branch("innerFullSecID", & innerFullSecID3sp);

      for (auto staticSector : m_staticSectors) {
        if (staticSector == nullptr)
          // Why there is an empty sector per layer?
          continue;


        outerFullSecID3sp = outerFullSecID2sp = staticSector->getFullSecID();
        auto segmentFilters = staticSector->getAllFilters2sp();
        for (auto compactIdFilterPair : segmentFilters) {
          auto innerCompactId = compactIdFilterPair.first;
          innerFullSecID2sp = getFullID(innerCompactId);
          twoHitFilter = compactIdFilterPair.second;
          sp2tree->Fill();
        }

        auto tripletFilters = staticSector->getAllFilters3sp();
        for (auto compactIdFilterPair : tripletFilters) {
          CompactSecIDs::sectorID_t id_center, id_inner;
          CompactSecIDs::extractCompactID(compactIdFilterPair.first, id_center, id_inner);
          centerFullSecID3sp = getFullID(id_center);
          innerFullSecID3sp = getFullID(id_inner);
          threeHitFilter = compactIdFilterPair.second;
          sp3tree->Fill();
        }


      }

      return true;
    }

    /// Retrieves from the current TDirectory the StaticSectors.
    bool retrieveStaticSectors(void)
    {
      TTree* sp2tree = new TTree("SegmentFilters", "SegmentFilters");
      twoHitFilter_t twoHitFilter;
      twoHitFilter.persist(sp2tree, "filter");

      unsigned int outerFullSecID2sp, innerFullSecID2sp;
      sp2tree->Branch("outerFullSecID", & outerFullSecID2sp);
      sp2tree->Branch("innerFullSecID", & innerFullSecID2sp);

      for (Long64_t i = 0 ; i < sp2tree->GetEntries() ; i++) {
        sp2tree->GetEntry(i);
        if (!addTwoHitFilter(outerFullSecID2sp, innerFullSecID2sp,
                             twoHitFilter))
          return false;

      }

      TTree* sp3tree = new TTree("TripletsFilters", "TripletFilters");
      threeHitFilter_t threeHitFilter;
      threeHitFilter.persist(sp3tree, "filter");

      unsigned int outerFullSecID3sp, centerFullSecID3sp,
               innerFullSecID3sp;
      sp3tree->Branch("outerFullSecID", & outerFullSecID3sp);
      sp3tree->Branch("centerFullSecID", & centerFullSecID3sp);
      sp3tree->Branch("innerFullSecID", & innerFullSecID3sp);

      for (Long64_t i = 0 ; i < sp3tree->GetEntries() ; i++) {
        sp3tree->GetEntry(i);
        if (!addThreeHitFilter(outerFullSecID3sp, centerFullSecID3sp,
                               innerFullSecID3sp,
                               threeHitFilter))
          return false;

      }

      return true;
    }

    /**
     * This member takes care of converting the [layer][ladder]
     * [sensor][sector] multi index into a linear index on
     * the m_staticSectors vector.
     */
    CompactSecIDs m_compactSecIDsMap;

    /** This vector contains all the static sectors on a sector map.
     *  The index is the compact ID provided by the CompactSecIDs
     */
    std::vector< staticSector_t* > m_staticSectors;

    /** Configuration: i.e. name of the sector map, tuning
    parameters, etc.  */
    SectorMapConfig m_testConfig;

  };

}


#endif
