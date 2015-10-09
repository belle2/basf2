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
#include <tuple>
#include <utility>

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
    std::vector< FullSecID                        > m_inner2spSecIDs;
    std::vector< std::pair< FullSecID, FullSecID> > m_inner3spSecIDs;
    std::vector< std::tuple< FullSecID, FullSecID, FullSecID > > m_inner4spSecIDs;

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
    void assign2spFilter(FullSecID inner, const Filter2sp  filter)
    {
      m_2spFilters[ m_compactSecIDsMap->getCompactID(inner) ] = filter;
      m_inner2spSecIDs.push_back(inner);
    }

    /** Parameters: pass the ID of the inner sectors (sorted from outer(left) to inner(right) and the filters you like to attach */
    void assign3spFilter(FullSecID center, FullSecID inner,
                         const Filter3sp  filter)
    {
      m_3spFilters[ m_compactSecIDsMap->getCompactID(center, inner) ] = filter;
      m_inner3spSecIDs.push_back({center, inner});
    }

    /** Parameters: pass the ID of the inner sectors (sorted from outer(left) to inner(right) and the filters you like to attach */
    void assign4spFilter(FullSecID outerCenter, FullSecID innerCenter, FullSecID inner,
                         const Filter3sp  filter)
    {
      m_4spFilters[ m_compactSecIDsMap->getCompactID(outerCenter, innerCenter, inner) ] =
        filter;
      m_inner4spSecIDs.push_back({outerCenter, innerCenter, inner});
    }



    /** returns innerSecIDs */
    const std::vector< FullSecID >& getInner2spSecIDs() const
    {
      return m_inner2spSecIDs;
    }


    /** returns innerSecIDs */
    const std::vector< std::pair<FullSecID, FullSecID> >& getInner3spSecIDs() const
    {
      return m_inner3spSecIDs;
    }

    /** returns innerSecIDs */
    const std::vector< std::tuple<FullSecID, FullSecID, FullSecID> >& getInner4spSecIDs() const
    {
      return m_inner4spSecIDs;
    }

    /** returns FullSecID of this sector */
    FullSecID getFullSecID() const { return m_secID; }


    /// COMPARISON OPERATORS

    /** == -operator - compares if two StaticSectors are identical */

    inline bool operator == (const StaticSector& b) const
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
    bool accept(FullSecID center, FullSecID inner,
                const HitType& outerSp, const HitType& centerSp, const HitType& innerSp) const
    {
      auto filter = m_3spFilters[m_compactSecIDsMap->getCompactID(center, inner) ].second;
      return filter.accept(outerSp , centerSp, innerSp);

    }

    /** applies all filters enabled for given combination of sectors
    * (this Sector and sector with passed fullSecID) on given hits,
    * sorting of parameters: from outer to inner, independently for SecID and Hits
    * returns true if accepted */
    bool accept(FullSecID outerCenter, FullSecID innerCenter, FullSecID inner,
                const HitType& outerSp, const HitType& outerCenterSp,
                const HitType& innerCenterSp, const HitType& innerSp) const
    {
      auto filter = m_4spFilters[m_compactSecIDsMap->getCompactID(outerCenter, innerCenter, inner) ].second;
      return filter.accept(outerSp , outerCenterSp, innerCenterSp, innerSp);

    }


  };


} //Belle2 namespace
