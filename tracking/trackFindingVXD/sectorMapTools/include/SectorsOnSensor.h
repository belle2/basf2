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

#include <assert.h>

#include <iostream>
using namespace std;
namespace Belle2 {


  // Manage to retrieve the sectorID from the normalized u,v coordinates
  template< class sectorID  >
  class SectorsOnSensor {

    typedef unsigned char index_t;

    map<float , index_t> m_normalizedUsup;
    map<float , index_t> m_normalizedVsup;

    vector< vector < FullSecID > > m_fullSecIDs; // indexed by U V
    vector< sectorID > m_compactSecIDs;          // indexed by FullSecID

  public:

    SectorsOnSensor() {}
    SectorsOnSensor(
      const vector< float >&      normalizedUsup,
      const vector< float >&      normalizedVsup,
      const vector< vector< FullSecID > >& fullSecIDs
    )
    {
      index_t index = 0;

      for (auto Vsup : normalizedVsup)
        m_normalizedVsup.insert(pair<float, index_t>(Vsup, index++));
      m_normalizedVsup.insert(pair<float, index_t>(FLT_MAX, index++));

      index = 0;
      for (auto Usup : normalizedUsup)
        m_normalizedUsup.insert(pair<float, index_t>(Usup, index++));
      m_normalizedUsup.insert(pair<float, index_t>(FLT_MAX, index++));

      m_fullSecIDs = fullSecIDs;

    }

    ~SectorsOnSensor() {};

    FullSecID operator()(float normalizedU, float normalizedV) const
    {
      //cout << "| " ;
      //for(auto u: m_normalizedUsup )

      //  cout<< "< " << u.first << " , " << (int) u.second << " >"  << "\t";
      //cout << "| "<< endl;

      auto uIndex = m_normalizedUsup.upper_bound(normalizedU)->second;
      auto vIndex = m_normalizedVsup.upper_bound(normalizedV)->second;

      cout << "( " << (int) uIndex << " " << (int) vIndex << " )";
      return m_fullSecIDs[ uIndex ][ vIndex ];
    }

    // minimal vector semantics to access the compactSecIDs vector
    sectorID& operator[](int index) { return m_compactSecIDs[index] ;};
    const sectorID& operator[](int index) const { return m_compactSecIDs[index] ;};

    size_t size() const { return m_compactSecIDs.size(); };
    void resize(size_t n) { m_compactSecIDs.resize(n); };

  };

}
