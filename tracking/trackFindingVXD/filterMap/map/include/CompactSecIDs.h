/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include "tracking/dataobjects/FullSecID.h"
#include <tracking/trackFindingVXD/filterMap/map/SectorsOnSensor.h>
#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
#include <cstdint>
#include <vector>

namespace Belle2 {

  /// This class provides a computer convenient numbering scheme for
  /// the sectors in the sector map and for the N sectors combinations.
  /// Sectors are labeled with consecutives numbers starting from 1.
  /// Methods are provided to convert the fullSecIds (human readable) to
  /// compactSecIDs (computer convenient) back and forth.
  class CompactSecIDs {

  public:

    // uint_16_t upper limit is UINT16_MAX

    /// Typedef of the compact Id for a single sector
    typedef uint16_t sectorID_t       ;

    /// Typedef of the compact Id for a 2 sectors combination
    typedef uint32_t secPairID_t      ;

    /// Typedef of the compact Id for a 3 sectors combination
    typedef uint64_t secTripletID_t   ;

    /// Typedef of the compact Id for a 4 sectors combination
    typedef uint64_t secQuadrupletID_t;

    /// The constructor have just to set the internal counter to 0.
    CompactSecIDs(): m_sectorCounter(0) { }

    /// The destructor is quite trivial: nothing special to delete.
    ~CompactSecIDs() {}

    /// Returns the number of sectors defined so far.
    int getSize() const {return (int)m_sectorCounter ; }

    /// Returns the available number of sector that can be defined now.
    unsigned int getAvailableSlots() const
    {
      return  UINT16_MAX - (getSize());
    }


    /// This method defines all the sectors on a given sensor.
    /// The two vectors normalizedUsup and normalizedVsup define the
    /// sector boundaries in normalized coordinates. (e.g.:
    /// if normalizesUsup = { a, 1} two sectors boundary are defined
    /// [0,a) and [a,1). )
    /// The returned value is the number of fullSecIDs actually added to
    /// this CompacSecIDs.
    int addSectors(const std::vector< double>&   normalizedUsup,
                   const std::vector< double>&   normalizedVsup,
                   const std::vector< std::vector< FullSecID >>& fullSecIDs)
    {
      // First let us check that there is space to store all the sectors
      if (getAvailableSlots() < (normalizedUsup.size() + 1) *
          (normalizedVsup.size() + 1))
        return 0;

      // Second let us check that the array of fullSecIDs is rectangular
      // and that its size matches the size of the coordinates.
      if (fullSecIDs.size() != normalizedUsup.size() + 1 ||
          fullSecIDs[0].size() != normalizedVsup.size() + 1)
        return 0;

      // Further sanity checks

      auto layer  = fullSecIDs[0][0].getLayerID();
      auto ladder = fullSecIDs[0][0].getLadderID();
      auto sensor = fullSecIDs[0][0].getVxdID().getSensorNumber();

      for (auto fullSecIDrow : fullSecIDs) {
        // Check that the fullSecIDs vector of vector is rectangular
        if (fullSecIDrow.size() != normalizedVsup.size() + 1)
          return 0;

        // Check that all the fullSectorIDs are on the same physical sensor
        for (auto fullSecID : fullSecIDrow)
          if (layer  != fullSecID.getLayerID() ||
              ladder != fullSecID.getLadderID() ||
              sensor != fullSecID.getVxdID().getSensorNumber())
            return 0;
      }

      return privateAddSectors(m_compactSectorsIDMap,
                               normalizedUsup, normalizedVsup,
                               fullSecIDs,
                               layer, ladder, sensor);

    }


    /** Returns the compact id of the FullSecID
        It does not throw exceptions (at least it should not).
      @param fullID: the FullSecID
      @return 0 if fullID is unknown, else the compact sector id
    */
    sectorID_t getCompactID(const FullSecID& fullID) const
    {

      return privateGetCompactID(m_compactSectorsIDMap,
                                 fullID.getLayerID(),
                                 fullID.getLadderID(),
                                 fullID.getVxdID().getSensorNumber(),
                                 fullID.getSecID());
    }

    /** Getter for IDs of all sectors on a sensor
     * @param layer : layer number of the sensor
     * @param ladder : ladder number of the sensor
     * @param sensor : sensor number
     * @return vector containing the IDs of the sectors on the sensor
     */
    SectorsOnSensor<sectorID_t>
    getSectorsOnSensor(unsigned layer, unsigned ladder, unsigned sensor)
    const
    {
      try {
        return m_compactSectorsIDMap.at(layer).at(ladder).at(sensor);
      } catch (...)
      { return SectorsOnSensor<sectorID_t>(); }

    }

    /// Exception for normalized coordinate U out of bound [0,1]
    BELLE2_DEFINE_EXCEPTION(unboundedNormalizedU,
                            "On layer:%1% ladder:%2% sensor:%3% abnormal U=%4$");

    /// Exception for normalized coordinate V out of bound [0,1]
    BELLE2_DEFINE_EXCEPTION(unboundedNormalizedV,
                            "On layer:%1% ladder:%2% sensor:%3% abnormal V=%4$");

    /// Returns a fullSecID for given sensor and pair of coordinates.
    /// An exception is thrown in case of errors (index/coordinate out of range).
    FullSecID getFullSecID(VxdID aSensorID,
                           double normalizedU, double normalizedV) const
    {

      auto layer = aSensorID.getLayerNumber() ;
      auto ladder = aSensorID.getLadderNumber();
      auto sensor = aSensorID.getSensorNumber();

      auto sectorsOnSensor =
        m_compactSectorsIDMap.at(layer).at(ladder).at(sensor);


      if (normalizedU < 0. || 1. < normalizedU)
        B2WARNING("CompactSecIDs: U not normalized! This may lead to undefined behavior!");
      if (normalizedV < 0. || 1. < normalizedV)
        B2WARNING("CompactSecIDs: V not normalized! This may lead to undefined behavior!");

      return sectorsOnSensor(normalizedU, normalizedV);


    }

    /// Returns the compact id of the FullSecID fullID.
    /// It will not throw exceptions.
    /// @param fullID the FullSecID
    /// @return if fullID is unknown 0 is returned, else the corresponding compact id
    sectorID_t operator [](const FullSecID& fullID) const
    {
      return getCompactID(fullID);
    }

    /// Returns the compact id of the pair of FullSecID id1 id2.
    /// @param id1 first FullSecID
    /// @param id2 second FullSecID
    /// @return if id1 or id2 are undefined 0 is returned, else the compact sector id
    secPairID_t getCompactID(const FullSecID& id1, const FullSecID& id2) const
    {

      auto i1 = (secPairID_t)getCompactID(id1);
      if (i1 == 0)
        return 0;

      auto i2 = (secPairID_t)getCompactID(id2);
      if (i2 == 0)
        return 0;

      return i1 + (i2 << 16);

    }

    /// Uses the values coded by the Sector Pair ID pair_id
    /// and sets the two compact Sector ids id1 and id2
    /// @param pair_id input sector pair id
    /// @param id1 reference to first sector id to be set
    /// @param id2 reference second sector id to be set
    static void extractCompactID(secPairID_t pair_id, sectorID_t& id1, sectorID_t& id2)
    {
      id1 = pair_id & 0xffff;
      id2 = (pair_id >> 16) & 0xffff;
    }



    /// Returns the compact id of the triplet of FullSecID id1 id2 id3
    /// @param id1: first id
    /// @param id2: second id
    /// @param id3: third id
    /// @return If any of the id1, id2 or id3 is undefined 0 is returned, else the compact id of the triplet
    secTripletID_t getCompactID(const FullSecID& id1, const FullSecID& id2,
                                const FullSecID& id3) const
    {

      auto i1 = (secTripletID_t)getCompactID(id1);
      if (i1 == 0)
        return 0;

      auto i2 = (secTripletID_t)getCompactID(id2);
      if (i2 == 0)
        return 0;

      auto i3 = (secTripletID_t)getCompactID(id3);
      if (i3 == 0)
        return 0;

      return i1 + (i2 << 16) + (i3 << 32);

    }

    /// Uses the values coded by the Sector Pair ID to set the three compact sector ids
    /// @param triplet_id input compact id of the triplet
    /// @param id1 reference to compact id of first sector to be set
    /// @param id2 reference to compact id of second sector to be set
    /// @param id3 reference to compact id of third sector to be set
    static void extractCompactID(secTripletID_t triplet_id, sectorID_t& id1, sectorID_t& id2, sectorID_t& id3)
    {
      id1 = triplet_id & 0xffff;
      id2 = (triplet_id >> 16) & 0xffff;
      id3 = (triplet_id >> 32) & 0xffff;
    }

    /// Returns the compact id of the quadruplet of FullSecID id1 id2 id3 id4
    /// If any of the id1, id2, id3 or id4 is undefined 0 is returned.
    secQuadrupletID_t getCompactID(const FullSecID& id1, const FullSecID& id2,
                                   const FullSecID& id3, const FullSecID& id4)
    const
    {

      auto i1 = (secQuadrupletID_t)getCompactID(id1);
      if (i1 == 0)
        return 0;

      auto i2 = (secQuadrupletID_t)getCompactID(id2);
      if (i2 == 0)
        return 0;

      auto i3 = (secQuadrupletID_t)getCompactID(id3);
      if (i3 == 0)
        return 0;

      auto i4 = (secQuadrupletID_t)getCompactID(id4);
      if (i4 == 0)
        return 0;

      return i1 + (i2 << 16) + (i3 << 32) + (i4 << 48) ;

    }

    /// Fast (and potentially dangerous) equivalent of getCompactID.
    /// It will not check for out of boundaries fullID / ill defined @param fullID
    sectorID_t getCompactIDFastAndDangerous(const FullSecID& fullID) const
    {

      auto layer = fullID.getLayerID();
      auto ladder = fullID.getLadderID();
      auto sensor = fullID.getVxdID().getSensorNumber();
      auto sector = fullID.getSecID();

      return m_compactSectorsIDMap[layer][ladder][sensor][sector];

    }



    /// JKL: returns true if operator() will not throw an exception
    bool areCoordinatesValid(VxdID aSensorID, double normalizedU, double normalizedV) const
    {
      auto layer = aSensorID.getLayerNumber() ;
      auto ladder = aSensorID.getLadderNumber();
      auto sensor = aSensorID.getSensorNumber();

      // check sensor:
      if (!(layer < m_compactSectorsIDMap.size())) return false;
      if (!(ladder < m_compactSectorsIDMap[layer].size())) return false;
      if (!(sensor < m_compactSectorsIDMap[layer][ladder].size())) return false;

      // check u and v:
      return m_compactSectorsIDMap[layer][ladder][sensor].areCoordinatesValid(normalizedU, normalizedV);
    }

    /// Typedef for vector of IDs of sectors on a sensors
    typedef std::vector<SectorsOnSensor<sectorID_t> > SensorsOnLadder_t;
    /// Typedef for vector of vector of IDs of sectors on a Ladder
    typedef std::vector<SensorsOnLadder_t> LaddersOnLayer_t;
    /// Typedef for vector of vector of vector of IDs of sectors on a layer
    typedef std::vector<LaddersOnLayer_t> LayersLookUpTable_t;


    /// Get access to the whole map
    const LayersLookUpTable_t& getCompactSectorsIDMap() const { return m_compactSectorsIDMap; }

    /// get the number of layers in this CompactSecIDs
    int nOfLayers(void) const
    { return m_compactSectorsIDMap.size(); }

    /** get the number of ladders on
      @param layer: the layer
      @return 0 if the the layer is out of bounds
    */
    int nOfLadders(int layer) const
    {
      if (layer < 0 || layer >= nOfLayers())
        return 0;
      return m_compactSectorsIDMap[layer].size();
    }

    /** get the number of sensors on
      @param layer: the layer
      @param ladder: the ladder
      @return 0 if the layer or ladder is out of bounds.
    */
    int nOfSensors(int layer, int ladder) const
    {
      if (ladder < 0 || ladder >= nOfLadders(layer))
        return 0; // Note: the check on the layer is implicitely made

      return m_compactSectorsIDMap[layer][ladder].size();
    }

    /// set the SublayerID of the sector
    /// @param sector: the FullSectorID of the sector, the sublayer id will be ignored during searching for the sector
    /// @param sublayer: the new sublayer id
    /// @return true if successful
    bool setSubLayerID(FullSecID& sector, int sublayer)
    {
      // cross check if sector is registered in the map, compactID will be 0 if not
      sectorID_t compactID = getCompactID(sector);
      if (compactID == 0) return false;

      // now update the SectorsOnSensor object
      return m_compactSectorsIDMap[sector.getLayerID()][sector.getLadderID()]
             [sector.getVxdID().getSensorNumber()].updateSubLayerID(sector, sublayer);
    }


  private:
    /// Counter for sectors
    sectorID_t m_sectorCounter;

    /// Lookup table containing all sectorIDs
    LayersLookUpTable_t m_compactSectorsIDMap;

    /// The hidden private method that recursively manage the size of everything.
    /// It returns the number of added sectors.
    /// It returns 0 in case of errors (memory exausted, or sector redefinition.)
    template<  class TContainer,
               class ... Indexes >
    int privateAddSectors(TContainer& container,
                          const std::vector< double >&     normalizedUsup,
                          const std::vector< double >&     normalizedVsup,
                          const std::vector< std::vector< FullSecID > >& fullSecIDs,
                          short unsigned int index, Indexes ... indexes)
    {
      try {
        if ((int) container.size() < (int)index + 1)
          container.resize(index + 1);
      } catch (...) { return 0; }
      return privateAddSectors(container[ index ],
                               normalizedUsup, normalizedVsup,
                               fullSecIDs, indexes...);
    }


    /// The hidden private method that end the recursion.
    /// It returns the number of added sectors.
    /// In particular it return 0 if sectors are already defined on the sensor.
    int privateAddSectors(SectorsOnSensor<sectorID_t>&            sectors,
                          const std::vector< double>&                  normalizedUsup,
                          const std::vector< double>&                  normalizedVsup,
                          const std::vector< std::vector< FullSecID > >&    fullSecIDs)
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

    /// The hidden private method that recursively descend the layer, ladder,
    /// sensor stack and returns the compactID of a given fullSecID.
    /// Sanity checks are performed at each step.
    /// 0 is returned in case of errors (i.e. indexes out of bound).
    template<  class TContainer,
               class ... Indexes >
    sectorID_t privateGetCompactID(const TContainer& container,
                                   short unsigned int index, Indexes ... indexes) const
    {
      if ((int) container.size() < (int) index + 1)
        return 0;

      return privateGetCompactID(container[ index ], indexes...);
    }


    /// The hidden private method that close the recursion.
    /// Sanity checks are performed on the last index.
    /// 0 is returned in case of errors (i.e. indexes out of bound).
    template<  class TContainer >
    sectorID_t privateGetCompactID(const TContainer& container,
                                   short unsigned int index) const
    {
      if ((int) container.size() < (int) index + 1)
        return 0;

      return container[ index ];
    }

  };

}
