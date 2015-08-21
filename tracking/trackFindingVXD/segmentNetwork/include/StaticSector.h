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

  template < class HitType,
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

    /** Parameters: pass the ID of the next inner sector and the filters you like to attach */
    void assign2spFilter(FullSecID sector, const Filter2sp  filter)
    {
      m_2spFilters[ m_compactSecIDsMap->getCompactID(sector) ] = filter;
    }

    /** Parameters: pass the ID of the inner sectors (sorted from outer(left) to inner(right) and the filters you like to attach */
    void assign3spFilter(FullSecID sector1, FullSecID sector2,
                         const Filter3sp  filter)
    {
      m_3spFilters[ m_compactSecIDsMap->getCompactID(sector1, sector2) ] = filter;
    }

    /** Parameters: pass the ID of the inner sectors (sorted from outer(left) to inner(right) and the filters you like to attach */
    void assign4spFilter(FullSecID sector1, FullSecID sector2, FullSecID sector3,
                         const Filter3sp  filter)
    {
      m_4spFilters[ m_compactSecIDsMap->getCompactID(sector1, sector2, sector3) ] =
        filter;
    }



    /** returns innerSecIDs */
    const std::vector<Belle2::FullSecID>& getInnerSecIDs() const
    {
      return m_innerSecIDs; // WARNING this is a dummy yet!
    }

    /** returns FullSecID of this sector */
    FullSecID getFullSecID() const { return m_secID; }


    /// COMPARISON OPERATORS

    /** == -operator - compares if two StaticSectors are identical */
    template< class StaticSectorType >
    inline bool operator == (const StaticSectorType& b) const
    {
      return (getFullSecID() == b.getFullSecID());
    }

    /** == -operator - compares if two StaticSectors are identical */
    inline bool operator == (const FullSecID& b) const
    {
      return (getFullSecID() == b);
    }




    /** applies all filters enabled for given combination of sectors
    * (this Sector and sector with passed fullSecID) on given hits,
    * sorting of parameters: from outer to inner, independently for SecID and Hits
    * returns true if accepted */
    bool accept(const FullSecID& innerSecID,
                const HitType& spOnThisSec,
                const HitType& spOnInnerSec) const
    {
      auto filter = m_2spFilters[ m_compactSecIDsMap->getCompactID(innerSecID) ].second;
      return filter.accept(spOnThisSec , spOnInnerSec);
    }

    /** applies all filters enabled for given combination of sectors
    * (this Sector and sector with passed fullSecID) on given hits,
    * sorting of parameters: from outer to inner, independently for SecID and Hits
    * returns true if accepted */
    bool accept(FullSecID, FullSecID, HitType&, HitType&, HitType&) const { return true; }

    /** applies all filters enabled for given combination of sectors
    * (this Sector and sector with passed fullSecID) on given hits,
    * sorting of parameters: from outer to inner, independently for SecID and Hits
    * returns true if accepted */
    bool accept(FullSecID, FullSecID, FullSecID, HitType&, HitType&, HitType&, HitType&) const { return true; }
  };


} //Belle2 namespace
