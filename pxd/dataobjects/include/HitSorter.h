/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef HITSORTER_H
#define HITSORTER_H

// stl
#include <set>

// boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace Belle2 {

//-----------------------------------------------------------------
//         StoreIndex and sets of StoreIndices.
//-----------------------------------------------------------------


  /** Index into StoreArray. */
  typedef unsigned int StoreIndex;

  /** Set of indices into StoreArray. */
  typedef std::set<StoreIndex> StoreIndexSet;
  typedef std::set<StoreIndex>::iterator StoreIndexSetItr;


  //-----------------------------------------------------------------
  //         A structure to slice hits according to sensors.
  //-----------------------------------------------------------------

  /** StoreRecord holds index of a hit in its StoreArray, and UniID of its detector.*/
  struct StoreRecord {
    int m_sensorUniID;
    StoreIndex m_index;
  };

  /** Type to store list of detectors hit in current event. */
  typedef std::set<int> SensorSet;
  typedef std::set<int>::iterator SensorSetItr;

  /** Index tag for access using hit number.*/
  struct StoreIndexSide {};

  /** Index tag for access using sensor UniID. */
  struct SensorUniIDSide {};

  /**
   * StoreRecordSet - a doubly-indexed set of StoreArray indices and sensor UIDs.
   *
   * The container is intended for optimization of processing by slicing sets
   * of hits by sensor UID, so that hits in a sensor can be processed together.
   * Intended use:
   *
   * StoreRecordSet hitSorter;
   * SensorUIDSet usedSensors;
   *
   * Fill the container:
   * (loop over hits)
   *    - fill StoreIndex and sensoru UID into hitSorter;
   *    - fill sensor UID into usedSensors;
   */

  typedef boost::multi_index_container <
  StoreRecord,
  boost::multi_index::indexed_by <
  boost::multi_index::ordered_unique <
  boost::multi_index::tag<StoreIndexSide>,
  boost::multi_index::member <
  StoreRecord, StoreIndex, &StoreRecord::m_index >
  > ,
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<SensorUniIDSide>,
  boost::multi_index::member <
  StoreRecord, int, &StoreRecord::m_sensorUniID
  >
  >
  >
  >  StoreRecordSet;


  // Typedefs for indices
  typedef StoreRecordSet::index<StoreIndexSide>::type StoreSideIndex;
  typedef StoreRecordSet::index<StoreIndexSide>::type::iterator StoreSideItr;
  typedef StoreRecordSet::index<SensorUniIDSide>::type SensorSideIndex;
  typedef StoreRecordSet::index<SensorUniIDSide>::type::iterator SensorSideItr;

} // namespace Belle2

#endif
