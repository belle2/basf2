/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/
#pragma once

#include "tracking/dataobjects/FullSecID.h"
#include "tracking/trackFindingVXD/sectorMapTools/SectorsOnSensor.h"
#include <cstdint>
#include <vector>



using namespace std;

namespace Belle2 {

  class CompactSecIDs {

  public:
    // limit to UINT16_MAX
    typedef uint16_t sectorID_t       ;
    typedef uint32_t secPairID_t      ;
    typedef uint64_t secTripletID_t   ;
    typedef uint64_t secQuadrupletID_t;

    ~CompactSecIDs() {}
    CompactSecIDs(): m_sectorCounter(0) { }

    int getSize() const {return (int)m_sectorCounter ; }
    unsigned int getAvailableSlots() const
    {
      return  UINT16_MAX - (getSize());
    }


    // add the FullSecIDs fullSecIDs to the internal lookup table
    // return the number of fullSecIDs actually added
    int addSectors(const vector< float>&   normalizedUsup,
                   const vector< float>&   normalizedVsup,
                   const vector< vector< FullSecID >>& fullSecIDs)
    {

      if (getAvailableSlots() < (normalizedUsup.size() + 1) *
          (normalizedVsup.size() + 1) ||
          fullSecIDs.size() != normalizedUsup.size() + 1 ||
          fullSecIDs[0].size() != normalizedVsup.size() + 1)
        return 0;

      // sanity checks

      auto layer  = fullSecIDs[0][0].getLayerID();
      auto ladder = fullSecIDs[0][0].getLadderID();
      auto sensor = fullSecIDs[0][0].getVxdID().getSensorNumber();

      for (auto fullSecIDrow : fullSecIDs) {
        if (fullSecIDrow.size() != normalizedVsup.size() + 1)
          return 0;

        for (auto fullSecID : fullSecIDrow)
          if (layer  != fullSecID.getLayerID() ||
              ladder != fullSecID.getLadderID() ||
              sensor != fullSecID.getVxdID().getSensorNumber())
            return 0;
      }

      return addSectors(m_compactSectorsIDMap,
                        normalizedUsup, normalizedVsup,
                        fullSecIDs,
                        layer, ladder, sensor);

    }


    sectorID_t getCompactID(const FullSecID& fullID) const
    {

      return getCompactID(m_compactSectorsIDMap,
                          fullID.getLayerID(),
                          fullID.getLadderID(),
                          fullID.getVxdID().getSensorNumber(),
                          fullID.getSecID());

    }

    secPairID_t getCompactID(const FullSecID id1, const FullSecID id2) const
    {

      return (secPairID_t)getCompactID(id1) +
             ((secPairID_t)getCompactID(id2) << 16);

    }

    secTripletID_t getCompactID(const FullSecID id1, const FullSecID id2 ,
                                const FullSecID id3) const
    {

      return
        (secTripletID_t) getCompactID(id1) +
        ((secTripletID_t) getCompactID(id2) << 16) +
        ((secTripletID_t) getCompactID(id3) << 32);

    }

    secQuadrupletID_t getCompactID(const FullSecID id1, const FullSecID id2 ,
                                   const FullSecID id3, const FullSecID id4) const
    {

      return
        (secQuadrupletID_t) getCompactID(id1) +
        ((secQuadrupletID_t) getCompactID(id2) << 16) +
        ((secQuadrupletID_t) getCompactID(id3) << 32) +
        ((secQuadrupletID_t) getCompactID(id4) << 48);

    }

    sectorID_t operator [](const FullSecID& fullID) const
    {
      return getCompactID(fullID);
    }

    sectorID_t getCompactIDFastAndDangerous(const FullSecID& fullID) const
    {

      auto layer = fullID.getLayerID();
      auto ladder = fullID.getLadderID();
      auto sensor = fullID.getVxdID().getSensorNumber();
      auto sector = fullID.getSecID();

      return m_compactSectorsIDMap[layer][ladder][sensor][sector];

    }

    FullSecID getFullID(VxdID aSensorID, float x, float y) const
    {

      auto layer = aSensorID.getLayerNumber() ;
      auto ladder = aSensorID.getLadderNumber();
      auto sensor = aSensorID.getSensorNumber();

      return m_compactSectorsIDMap[layer][ladder][sensor](x, y);

    }


  private:

    sectorID_t m_sectorCounter;

    typedef vector< SectorsOnSensor<sectorID_t> > SensorsOnLadder_t;
    typedef vector< SensorsOnLadder_t > LaddersOnLayer_t;
    typedef vector< LaddersOnLayer_t > LayersLookUpTable_t;

    LayersLookUpTable_t m_compactSectorsIDMap;

    template<  class TContainer,
               class ... Indexes >
    int addSectors(TContainer& container,
                   const vector< float >&     normalizedUsup,
                   const vector< float >&     normalizedVsup,
                   const vector< vector< FullSecID > >& fullSecIDs,
                   short unsigned int index, Indexes ... indexes)
    {
      try {
        if ((int) container.size() < (int)index + 1)
          container.resize(index + 1);
      } catch (...) { return 0; }
      return addSectors(container[ index ], normalizedUsup, normalizedVsup,
                        fullSecIDs, indexes...);
    }



    int addSectors(SectorsOnSensor<sectorID_t>&            sectors,
                   const vector< float>&                   normalizedUsup,
                   const vector< float>&                   normalizedVsup,
                   const vector< vector< FullSecID > >&    fullSecIDs)
    {
      if (sectors.size() != 0)
        return 0;

      sectors = SectorsOnSensor< sectorID_t >(normalizedUsup,
                                              normalizedVsup, fullSecIDs);

      int addedSectors = 0;

      for (auto sectorRow : fullSecIDs)
        for (auto sector : sectorRow) {
          auto secID = sector.getSecID();
          try {
            if ((int) sectors.size() < (int) secID + 1)
              sectors.resize(secID + 1);
          } catch (...) { return addedSectors; }

          sectors[ secID ] = ++m_sectorCounter ;
          addedSectors++;
        }
      return addedSectors;

    }

    template<  class TContainer,
               class ... Indexes >
    sectorID_t getCompactID(const TContainer& container,
                            short unsigned int index, Indexes ... indexes) const
    {
      if ((int) container.size() < (int) index + 1)
        return 0;

      return getCompactID(container[ index ], indexes...);
    }


    template<  class TContainer >
    sectorID_t getCompactID(const TContainer& container,
                            short unsigned int index) const
    {
      if ((int) container.size() < (int) index + 1)
        return 0;

      return container[ index ];
    }


  };

}
