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
#include <tracking/dataobjects/FullSecID.h>
#include <tracking/trackFindingVXD/sectorMapTools/CompactSecIDs.h>

#include <unordered_map>
#include <iostream>

namespace Belle2 {

  template < class SpacePoint,
             class Filter2sp, class Filter3sp, class Filter4sp >
  class StaticSector {

/// DATA MEMBERS

  private:
    /** stores its own secID */
    FullSecID m_secID;

    const CompactSecIDs* m_compactSecIDsMap;

    /** stores innerSecIDs */
    std::vector<FullSecID> m_innerSecIDs;

    std::unordered_map<CompactSecIDs::sectorID_t    , Filter2sp > m_2spFilters;
    std::unordered_map<CompactSecIDs::secPairID_t   , Filter3sp > m_3spFilters;
    std::unordered_map<CompactSecIDs::secTripletID_t, Filter4sp > m_4spFilters;

  public:

    /// CONSTRUCTORS

    /** standard constructor */
    StaticSector() : m_secID(FullSecID()) { }

    /** constructor */
    StaticSector(FullSecID secID) : m_secID(secID) {}


    /// ACCESS FUNCTIONS

    Filter2sp getFilter2sp(FullSecID innerSector) const
    {
      static Filter2sp just_in_case;

      auto filter = m_2spFilters.find(m_compactSecIDsMap->getCompactID(innerSector));
      if (filter != m_2spFilters.end())
        return filter->second;
      else
        std::cout << " ??? " << std::endl;
      return just_in_case;
    }

    /** assign the 2 space points filter */
    void assignCompactSecIDsMap(const CompactSecIDs& compactSecIDsMap)
    {
      m_compactSecIDsMap = & compactSecIDsMap;
    }

    void assign2spFilter(FullSecID sector, const Filter2sp  filter)
    {
      m_2spFilters[ m_compactSecIDsMap->getCompactID(sector) ] = filter;
    }

    void assign3spFilter(FullSecID sector1, FullSecID sector2,
                         const Filter3sp  filter)
    {
      m_3spFilters[ m_compactSecIDsMap->getCompactID(sector1, sector2) ] = filter;

    }

    void assign3spFilter(FullSecID sector1, FullSecID sector2, FullSecID sector3,
                         const Filter3sp  filter)
    {
      m_4spFilters[ m_compactSecIDsMap->getCompactID(sector1, sector2, sector3) ] =
        filter;

    }



    /** returns innerSecIDs */
    const std::vector<Belle2::FullSecID>& getInnerSecIDs() const
    {
      return m_innerSecIDs;
    }

    /** returns FullSecID of this sector */
    FullSecID getFullSecID() const { return m_secID; }


    /// COMPARISON OPERATORS

    /** == -operator - compares if two StaticSectors are identical */
    template< class StaticSectorDummy >
    inline bool operator == (const StaticSectorDummy& b) const
    {
      return (getFullSecID() == b.getFullSecID());
    }

    /** == -operator - compares if two StaticSectors are identical */
    inline bool operator == (const FullSecID& b) const
    {
      return (getFullSecID() == b);
    }




    /** applies all filters enabled for combination of sectors
    (this Sector and sector with passed fullSecID on given spacePoints,
    returns true if accepted */
    bool accept(const FullSecID& outerSecID,
                const SpacePoint& spOnThisSec,
                const SpacePoint& spOnTheOuterSec) const
    {
      auto filter = m_2spFilters[ m_compactSecIDsMap->getCompactID(outerSecID) ].second;
      return filter.accept(spOnTheOuterSec , spOnTheOuterSec);
    }

    /** applies all filters enabled for combination of sectors (this Sector and sector with passed fullSecID on given spacePoints, returns true if accepted */
    template<class HitType>
    bool accept(FullSecID, FullSecID, HitType&, HitType&, HitType&) const { return true; }

  };


} //Belle2 namespace
