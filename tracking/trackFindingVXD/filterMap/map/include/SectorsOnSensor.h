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

namespace Belle2 {


  /// This class associates to an ordered pairs of normalized local coordinates
  /// a compact sector id.
  template< class sectorID  >
  class SectorsOnSensor {

    /// Typedef for the internal numbering of rows and columns of the sectors.
    typedef unsigned char index_t;

    /// Upper limits of the sectors in normalized U coordinates.
    std::map<double , index_t> m_normalizedUsup;

    /// Upper limits of the sectors in normalized V coordinates.
    std::map<double , index_t> m_normalizedVsup;

    /// The 2D array of the full sec ID is stored in this member.
    /// It is indexed by discretized normalized U V pairs
    std::vector< std::vector < FullSecID > > m_fullSecIDs;

    /// The 1D array of the compact ID is stored in this member.
    /// It is indexed by the sector component on the FullSecID
    std::vector< sectorID > m_compactSecIDs;

  public:

    /// Default constructor needed for the vector traits
    SectorsOnSensor() {}

    /// Useful constructor
    SectorsOnSensor(
      const std::vector< double >&      normalizedUsup,
      const std::vector< double >&      normalizedVsup,
      const std::vector< std::vector< FullSecID > >& fullSecIDs)
    {
      index_t index = 0;

      for (auto Vsup : normalizedVsup)
        m_normalizedVsup.insert({Vsup, index++});
      m_normalizedVsup.insert({FLT_MAX, index++});

      index = 0;
      for (auto Usup : normalizedUsup)
        m_normalizedUsup.insert({Usup, index++});
      // cppcheck-suppress unreadVariable
      m_normalizedUsup.insert({FLT_MAX, index++});

      m_fullSecIDs = fullSecIDs;

    }

    /// Destructor of the object
    ~SectorsOnSensor() {};

    /// Returns the Full Sector ID of the sector on this sensor that
    /// contains the point at normalized coordinates U, V
    FullSecID operator()(double normalizedU, double normalizedV) const
    {

      if (normalizedU < 0. or normalizedU > 1.)
        return FullSecID(0);
      if (normalizedV < 0. or normalizedV > 1.)
        return FullSecID(0);

      auto uKeyVal = m_normalizedUsup.upper_bound(normalizedU);
      if (uKeyVal == m_normalizedUsup.end())
        return FullSecID(0);

      auto vKeyVal = m_normalizedVsup.upper_bound(normalizedV);
      if (vKeyVal == m_normalizedVsup.end())
        return FullSecID(0);


      auto uIndex = uKeyVal->second;
      auto vIndex = vKeyVal->second;

      return m_fullSecIDs[ uIndex ][ vIndex ];
    }




    /// copy the vector members on the vector pointed from the arguments.
    ///  @param normalizedUsup
    ///  @param normalizedVsup
    ///  @param secID
    void get(std::vector< double >* normalizedUsup,
             std::vector< double >* normalizedVsup,
             std::vector< std::vector< unsigned int > >* secID) const
    {
      // let us copy the sorted map Usup
      for (auto uIndexPair : m_normalizedUsup)
        if (uIndexPair.first != FLT_MAX)
          normalizedUsup->push_back(uIndexPair.first);

      // let us copy the sorted map Vsup
      for (auto vIndexPair : m_normalizedVsup)
        if (vIndexPair.first != FLT_MAX)
          normalizedVsup->push_back(vIndexPair.first);

      // and finally we copy the array of full sec ids
      for (auto col : m_fullSecIDs) {
        std::vector< unsigned int > tmp_col;
        for (auto id : col)
          tmp_col.push_back(id);
        secID->push_back(tmp_col);
      }
    }

    /// check if using operator() would be safe (true if it is safe):
    bool areCoordinatesValid(double normalizedU, double normalizedV) const
    {
      // check u and v
      if ((normalizedU < 0.) or (normalizedU > 1.)) return false;
      if ((normalizedV < 0.) or (normalizedV > 1.)) return false;
      // check internal map for problems:
      if (m_normalizedUsup.upper_bound(normalizedU) == m_normalizedUsup.end())
        return false;

      if (m_normalizedVsup.upper_bound(normalizedV) == m_normalizedVsup.end())
        return false;

      return true;
    }


    /// minimal vector semantics to access the compactSecIDs vector
    /// using the sector @param index from the fullSecId
    sectorID& operator[](int index) { return m_compactSecIDs[index] ;};

    /// minimal vector semantics to access the compactSecIDs vector
    const sectorID& operator[](int index) const { return m_compactSecIDs[index] ;};

    /// minimal vector semantics to get the size of the compactSecIDs vector
    size_t size() const { return m_compactSecIDs.size(); };

    /// minimal vector semantics to resize the compactSecIDs vector
    void resize(size_t n) { m_compactSecIDs.resize(n); };


    /// update the sublayer id for the sector with the given FullSecID, the sublayer id is ignored when searching for the sector
    /// @param sector: FullSecID of the sector to be updated
    /// @param sublayer : the new value for the sublayer ID, the new SubLayerID will be 0 if sublayer==0, and will be 1 else
    bool updateSubLayerID(FullSecID sector, int sublayer)
    {
      for (auto& v : m_fullSecIDs) {
        for (FullSecID& thisSecID : v) {
          /// WARNING: the comparison will ignore the sublayer id
          if (sector.equalIgnoreSubLayerID(thisSecID)) {
            thisSecID = FullSecID(thisSecID.getVxdID(), (bool)sublayer, thisSecID.getSecID());
            return true;
          }
        }
      }
      return false;
    }

    /// JKL: for testing - get all compactSecIDs:
    const std::vector< sectorID >& getCompactSecIDs() const { return m_compactSecIDs; }

  };

}
