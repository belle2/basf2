/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// stl:
#include <vector>
#include <list>
#include <utility> // std::pair


namespace Belle2 {

  namespace VXDTFRawSecMapTypedef { // all typedefs for tracking/dataobjects/VXDTFRawSecMap.h

    // VXDTFRawSecMap:
    typedef std::list< double >
    CutoffValues; /**< contains all cutoff-Values collected during run, is a list since it has to be sorted in the end */
    typedef std::pair<unsigned int, CutoffValues> CutoffPack; /**< .first is code of filter, .second is CutoffValue */
    typedef std::vector< CutoffPack > FriendValues; /**< stores all Cutoffs */
    typedef std::pair<unsigned int, FriendValues > FriendPack; /**< .first is secID of current Friend, second is FriendValue */
    typedef std::vector< FriendPack > SectorValues; /**< stores all Friends */
    typedef std::pair<unsigned int, SectorValues> SectorPack; /**< .first is secID of current sector, second is SectorValue */
    typedef std::vector < SectorPack >
    StrippedRawSecMap; /**< stores all Sectors and a raw version of the data (no calculated cutoffs yet)*/
    typedef std::vector< unsigned int > IDVector; /**< stores IDs of sectors or friends or layerIDs */
    typedef std::pair<unsigned int, double> SectorDistance; /**< stores distance to origin (.second) for sector (.first) */
    typedef std::vector< SectorDistance > SectorDistancesMap; /**< stores vector of SectorDistanceInfo */
    // well, without typedef this would be:
    // vector< pair< unsigned int, vector< pair< unsigned int, vector< pair< unsigned int, vector<double> > > > > > > .... hail typedefs -.-
  }

  namespace VXDTFSecMapTypedef { // all typedefs for tracking/dataobjects/VXDTFSecMap.h

    // VXDTFSecMap:
    typedef std::pair< double, double> CutoffValue; /**< .first is minValue, .second is maxValue of a single cutoff */
    typedef std::pair<unsigned int, CutoffValue> Cutoff; /**< .first is code of filter, .second is CutoffValue */
    typedef std::vector< Cutoff > FriendValue; /**< stores all Cutoffs */
    typedef std::pair<unsigned int, FriendValue > Friend; /**< .first is secID of current Friend, second is FriendValue */
    typedef std::vector< Friend > SectorValue; /**< stores all Friends */
    typedef std::pair<unsigned int, SectorValue> Sector; /**< .first is secID of current sector, second is SectorValue */
    typedef std::vector < Sector > SecMapCopy; /**< stores all Sectors */
    // well, without typedef this would be:
    // vector< pair< unsigned int, vector< pair< unsigned int, vector< pair< unsigned int, pair<double, double> > > > > > > .... hail typedefs -.-
  }

  namespace PassDataTypedef {
    typedef std::pair<bool, double>
    Filter; /**< defines whether current filter is allowed (.first) and whether the cutoffvalues shall be tuned (.second).  */
  }
} //Belle2 namespace
