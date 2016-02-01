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
#include <map>
#include <vector>
#include <cfloat>
#include <utility>      // std::pair

#include <assert.h>

#include <framework/logging/Logger.h>
#include <framework/core/FrameworkExceptions.h>

namespace Belle2 {


  // Manage to retrieve the sectorID from the normalized u,v coordinates
  template< class sectorID  >
  class SectorsOnSensor {

    typedef unsigned char index_t;

    std::map<double , index_t> m_normalizedUsup;
    std::map<double , index_t> m_normalizedVsup;

    std::vector< std::vector < FullSecID > > m_fullSecIDs; // indexed by U V
    std::vector< sectorID > m_compactSecIDs;          // indexed by FullSecID

  public:

    /** gets thrown if coordinates are not within allowed boundaries. */
    BELLE2_DEFINE_EXCEPTION(Bad_coordinate, "SectorsOnSensor - coordinate %1% with value %2% is not within allowed range of [0;1]!");

    /** gets thrown if there is no sector stored for given coordinate value. */
    BELLE2_DEFINE_EXCEPTION(Sector_not_found, "SectorsOnSensor - could not find sector with coordinate %1% of value %2%!");

    SectorsOnSensor() {}
    SectorsOnSensor(
      const std::vector< double >&      normalizedUsup,
      const std::vector< double >&      normalizedVsup,
      const std::vector< std::vector< FullSecID > >& fullSecIDs
    )
    {
      index_t index = 0;

      for (auto Vsup : normalizedVsup)
        m_normalizedVsup.insert({Vsup, index++});
      m_normalizedVsup.insert({FLT_MAX, index++});

      index = 0;
      for (auto Usup : normalizedUsup)
        m_normalizedUsup.insert({Usup, index++});
      m_normalizedUsup.insert({FLT_MAX, index++});

      m_fullSecIDs = fullSecIDs;

    }

    ~SectorsOnSensor() {};

    FullSecID operator()(double normalizedU, double normalizedV) const
    {
      //cout << "| " ;
      //for(auto u: m_normalizedUsup )

      //  cout<< "< " << u.first << " , " << (int) u.second << " >"  << "\t";
      //cout << "| "<< endl;

      B2DEBUG(1, " normalizedU/V: " << normalizedU << "/" << normalizedV);
      // assert for out_of_range-cases:
      if (normalizedU < 0. or normalizedU > 1.) throw(Bad_coordinate() << "U" << normalizedU);
      if (normalizedV < 0. or normalizedV > 1.) throw(Bad_coordinate() << "V" << normalizedV);
// // //    assert(( "normalized U-coordinate must be in range [0;1]!" , !(normalizedU < 0.))); // and !(normalizedU > 1.));
// // //    assert(( "normalized U-coordinate must be in range [0;1]!" , !(normalizedU > 1.)));
// // //    assert(( "normalized V-coordinate must be in range [0;1]!" , !(normalizedV < 0.))); //  and !(normalizedV > 1.));
// // //    assert(( "normalized V-coordinate must be in range [0;1]!" , !(normalizedV > 1.)));

      // asert for cases when there is no relevant key found
      auto uKeyVal = m_normalizedUsup.upper_bound(normalizedU);
      if (uKeyVal == m_normalizedUsup.end()) throw(Sector_not_found() << "U" << normalizedU);
// // //    assert(uKeyVal != m_normalizedUsup.end());
      auto vKeyVal = m_normalizedVsup.upper_bound(normalizedV);
      if (vKeyVal == m_normalizedVsup.end()) throw(Sector_not_found() << "V" << normalizedV);
// // //    assert(vKeyVal != m_normalizedVsup.end());

      auto uIndex = uKeyVal->second;
      auto vIndex = vKeyVal->second;

      B2DEBUG(1, "( " << (int) uIndex << " " << (int) vIndex << " )");
      return m_fullSecIDs[ uIndex ][ vIndex ];
    }


    /// check if using operator() would be safe (true if it is safe):
    bool areCoordinatesValid(double normalizedU, double normalizedV) const
    {
      // check u and v
      if ((normalizedU < 0.) or (normalizedU > 1.)) return false;
      if ((normalizedV < 0.) or (normalizedV > 1.)) return false;
      // check internal map for problems:
      if (m_normalizedUsup.upper_bound(normalizedU) == m_normalizedUsup.end()) return false;
      if (m_normalizedVsup.upper_bound(normalizedV) == m_normalizedVsup.end()) return false;
      return true;
    }


    // minimal vector semantics to access the compactSecIDs vector
    sectorID& operator[](int index) { return m_compactSecIDs[index] ;};
    const sectorID& operator[](int index) const { return m_compactSecIDs[index] ;};

    size_t size() const { return m_compactSecIDs.size(); };
    void resize(size_t n) { m_compactSecIDs.resize(n); };

    /// JKL: for testing - get all compactSecIDs:
    const std::vector< sectorID >& getCompactSecIDs() const { return m_compactSecIDs; }
  };

}
