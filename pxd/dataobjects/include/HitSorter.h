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

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/logging/Logger.h>

// boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace Belle2 {
//-----------------------------------------------------------------
//         A structure to slice hits according to sensors.
//-----------------------------------------------------------------

  /** Index into StoreArray. */
  typedef unsigned int StoreIndex;

  /** Set of indices into StoreArray. */
  typedef std::set<StoreIndex> PXDSimHitSet;
  typedef std::set<StoreIndex>::iterator PXDSimHitSetItr;

  /** HitRecord holds index of a hit in its StoreArray, and UID of its detector.*/
  struct HitRecord {
    int m_sensorUID;
    StoreIndex m_index;
  };

  /** Type to store list of detectors hit in current event. */
  typedef std::set<int> SensorSet;
  typedef std::set<int>::iterator SensorSetItr;

  /** Index tag for access using hit number.*/
  struct HitIndexSide {};

  /** Index tag for access using sensor UID. */
  struct SensorUIDSide {};

  /**
   * HitRecordSet - a doubly-indexed set of StoreArray indices and sensor UIDs.
   *
   * The container is intended for optimization of processing by slicing sets
   * of hits by sensor UID, so that hits in a sensor can be processed together.
   * Intended use:
   *
   * HitRecordSet hitSorter;
   * SensorUIDSet usedSensors;
   *
   * Fill the container:
   * (loop over hits)
   *    - fill StoreIndex and sensoru UID into hitSorter;
   *    - fill sensor UID into usedSensors;
   */

  typedef boost::multi_index_container <
  HitRecord,
  boost::multi_index::indexed_by <
  boost::multi_index::ordered_unique <
  boost::multi_index::tag<HitIndexSide>,
  boost::multi_index::member <
  HitRecord, StoreIndex, &HitRecord::m_index >
  > ,
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<SensorUIDSide>,
  boost::multi_index::member <
  HitRecord, int, &HitRecord::m_sensorUID
  >
  >
  >
  >  HitRecordSet;


  // Typedefs for indices
  typedef HitRecordSet::index<HitIndexSide>::type HitSideIndex;
  typedef HitRecordSet::index<HitIndexSide>::type::iterator HitSideItr;
  typedef HitRecordSet::index<SensorUIDSide>::type SensorSideIndex;
  typedef HitRecordSet::index<SensorUIDSide>::type::iterator SensorSideItr;

}

#endif
