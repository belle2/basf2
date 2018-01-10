/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni, Thomas Lueck                                  *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/
#pragma once
#include <tracking/dataobjects/FullSecID.h>
#include <tracking/trackFindingVXD/filterMap/map/CompactSecIDs.h>
#include <tracking/trackFindingVXD/filterMap/filterFramework/TBranchLeafType.h>

#include <TF1.h>

#include <unordered_map>
#include <tuple>
#include <utility>

namespace Belle2 {

  template < class HitType,
             class Filter2sp, class Filter3sp, class Filter4sp >
  class StaticSector {

    // allows VXDTFFilters to modify private members and use private functions, which is kind of dangerous
    // maybe better to make the one function needed to modify the filters public
    template<class T>
    friend class VXDTFFilters;

/// PRIVATE MEMBERS

  private:

    /** function that modifies all 2SP-filters connected to this static sector
        @param adjustFunctions : a vector of vector of strings, the second vector is assumed to have 2 entries only.
          The first entry is the index of variable to change (see Filter::getNameAndReference where the indizes are defined)
          so it must be able to be converted to an int. The second is a regex to be used in a TF1 (root function).
    */
    void modify2SPFilters(const std::vector< std::vector< std::string > >& adjustFunctions)
    {
      // loop over all 2SP-filters
      for (auto& filter : m_2spFilters) modifySingleFilter<Filter2sp>(filter.second, adjustFunctions);
    }

    /** function that modifies all 3SP-filters connected to this static sector
        @param adjustFunctions : a vector of vector of strings, the second vector is assumed to have 2 entries only.
          The first entry is the index of variable to change (see Filter::getNameAndReference where the indizes are defined)
          so it must be able to be converted to an int. The second is a regex to be used in a TF1 (root function).
    */
    void modify3SPFilters(const std::vector< std::vector< std::string > >& adjustFunctions)
    {
      // loop over all 2SP-filters
      for (auto& filter : m_3spFilters) modifySingleFilter<Filter3sp>(filter.second, adjustFunctions);
    }

    /** Function that modifies the uper and lower bounds of the Ranges contained in the filter. The actual bounds
        which will be modified are accessed by their index they have (see Filter::getNameAndReference where the indizes are defined).
        @param filter: a specialization of Filter
        @param adjustFunctions : a vector of vector of strings, the second vector is assumed to have 2 entries only.
              The first entry is the index of variable to change (see Filter::getNameAndReference where the indizes are defined)
              so it must be able to be converted to an int. The second is a regex to be used in a TF1 (root function).
              For the regex it is assumed that "x" is the cutvalue itself and "y" is the FullSecID of the static sector (e.g. "sin(x)")
    */
    template<class FilterType>
    void modifySingleFilter(FilterType& filter, const std::vector< std::vector< std::string > >& adjustFunctions)
    {
      // get the "map" to the cut values, the char in the pair codes the type, and the pointer points to the value
      std::vector< std::pair<char, void*> > accessor = {};
      filter.getNameAndReference(&accessor);

      // this will produce lots of output
      B2DEBUG(1, std::endl << "BEFORE: " << filter.getNameAndReference() << std::endl);

      // loop over all adjustfunctions
      for (const std::vector< std::string >& entry : adjustFunctions) {
        if (entry.size() != 2) {
          B2FATAL("Incorrect format of input functions! Assumed format is vector<vector<string>> where the second (inside) vector"
                  << " only should contain exactly two entries!");
        }

        // asume first string is an integer
        int index = -1;
        try {
          index = std::stoi(entry[0]);
        } catch (...) {
          B2FATAL("Unable to perform stoi conversion! The provided string has to be able to be converted to an int (e.g. 1, 3, 42 ...). ");
        }

        if (index < 0 || index >= (int)accessor.size()) {
          B2FATAL("Provided index is out of range! index = " << index << " number of entries = " << accessor.size());
        }

        // now do some casting magic
        double x = 0;
        // the secID will be treated as 0th parameter of the TF1 ([0]) if specified.
        double y = m_secID;

        char typeID = accessor[index].first;
        void* valuePtr = accessor[index].second;
        if (typeID == TBranchLeafType(double())) x = *((double*)valuePtr);
        else if (typeID == TBranchLeafType(int())) x = *((int*)valuePtr);
        else if (typeID == TBranchLeafType(float())) x = *((float*)valuePtr);
        else if (typeID == TBranchLeafType(bool())) x = *((bool*)valuePtr);
        else {
          B2FATAL("Unrecognized type : " << typeID);
        } // end else if

        // create function
        TF1 f("function", entry[1].c_str());
        if (!f.IsValid() || f.GetNpar() > 1) {
          B2FATAL("No valid function provided! The provided string has to be able to be converted by TF1. Also max. 1 parameter is allowed!");
        }

        double result = f.EvalPar(&x, &y);

        // now cast back to original type and set the value
        if (typeID == TBranchLeafType(double())) *((double*)valuePtr) = result;
        else if (typeID == TBranchLeafType(int())) *((int*)valuePtr) = result;
        else if (typeID == TBranchLeafType(float())) *((float*)valuePtr) = result;
        else if (typeID == TBranchLeafType(bool())) *((bool*)valuePtr) = result;
        else {
          B2FATAL("Unrecognized type : " << typeID);
        } // end else if

      }// end loop over the functions

      B2DEBUG(1, "AFTER: " << filter.getNameAndReference() << std::endl);
    }

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
    StaticSector() : m_secID(FullSecID()), m_compactSecIDsMap(nullptr) { }

    /** constructor */
    StaticSector(FullSecID secID) : m_secID(secID), m_compactSecIDsMap(nullptr) {}


    // ACCESSOR FUNCTIONS

    /** Get the pionter to the 2 Space Point filter assigned to the
    friendship relation among this sector; will return NULL if filter is not found
    @param innerSector one */
    const Filter2sp* getFilter2sp(FullSecID innerSector) const
    {
      auto filter = m_2spFilters.find(m_compactSecIDsMap->getCompactID(innerSector));
      if (filter == m_2spFilters.end()) {
        return NULL;
      }
      return &(filter->second);
    }

    /** Get the pionter to the 3 Space Point filter assigned to the
    friendship relation among this sector; will return NULL if filter is not found
    @param centerID : FullSecID for the sector between the inner and this sector
    @param innerID : FullSecID for the inner sector*/
    const Filter3sp* getFilter3sp(const FullSecID& centerID, const  FullSecID& innerID) const
    {
      auto filter = m_3spFilters.find(m_compactSecIDsMap->getCompactID(centerID, innerID));
      if (filter == m_3spFilters.end()) {
        B2DEBUG(1, "StaticSector:getFilter3sp: could not find compactID for given SecIDs  (c/i: " << centerID.getFullSecString() <<
                "/"  << innerID.getFullSecString() << ")! Returning false.");
        return NULL;
      }
      return &(filter->second);
    }


    /** Get the pionter to the 4 Space Point filter assigned to the
    WARNING: not implemented yet. Will return a NULL pointer in any case!
    */
    const Filter4sp* getFilter4sp(const FullSecID& /*outerCenterID*/, const FullSecID& /*innerCenterID*/,
                                  const FullSecID& /*innerID*/) const
    {
      B2WARNING("StaticSector:getFilter4sp  4 hit, all 4 hits are yet ignored in here! TODO: implement!");
      return NULL;
    }



    /** Get constant access to the whole set of 2 Space Point filters.
     */
    const std::unordered_map<CompactSecIDs::sectorID_t    , Filter2sp >&
    getAllFilters2sp() const
    {return m_2spFilters;}

    /** Get constant access to the whole set of 3 Space Point filters.
     */
    const std::unordered_map<CompactSecIDs::secPairID_t   , Filter3sp >&
    getAllFilters3sp() const
    {return m_3spFilters;}

    /** Get constant access to the whole set of 4 Space Point filters.
     */
    const std::unordered_map<CompactSecIDs::secTripletID_t   , Filter4sp >&
    getAllFilters4sp() const
    {return m_4spFilters;}


    /** Assign the compact sector ID to this sector */
    void assignCompactSecIDsMap(const CompactSecIDs& compactSecIDsMap)
    {
      m_compactSecIDsMap = & compactSecIDsMap;
    }


    /** Assign the 2 space point @param filter to this static sector
    which is friend of the sector whose FullSecID is @param inner. */
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


    /** set sublayer ID, needed as it is updated in the trainings phase */
    // @param sublayer : the new SubLayerID will be 0 if sublayer==0, and will be 1 else
    void setSubLayerID(int sublayer)
    {
      m_secID = FullSecID(m_secID.getVxdID(), (bool)sublayer, m_secID.getSecID());
    }


    /** returns all IDs for inner sectors of two-sector-combinations */
    const std::vector< FullSecID >& getInner2spSecIDs() const
    {
      return m_inner2spSecIDs;
    }


    /** returns all IDs for inner sectors of three-sector-combinations */
    const std::vector< std::pair<FullSecID, FullSecID> >& getInner3spSecIDs() const
    {
      return m_inner3spSecIDs;
    }

    /** returns all IDs for inner sectors of four-sector-combinations */
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




  };


} //Belle2 namespace
