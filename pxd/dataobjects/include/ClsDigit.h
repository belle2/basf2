/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef CLSDIGIT_H
#define CLSDIGIT_H

#include <pxd/dataobjects/HitSorter.h>

#include <set>

// boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace Belle2 {

  /**
   * ClsDigitRec: A structure to carry digit data for clustering.
   */
  struct ClsDigitRecord {
    int m_uCellID;             /**< Cell number in u. */
    int m_vCellID;             /**< Cell number in v. */
    float m_charge;             /**< Charge accumulated in the cell. */
    StoreIndex m_index;        /**< PXDDigit's index in the StoreArray. */
    unsigned short m_cluster;  /**< Cluster number this digit belongs to; 0 - no cluster. */
  };

  /**
   * ClsDigitSet - a multi-index structure used in clustering.
   *
   * The structure provides several indices for clustering:
   * 1. CellSide index, which is a composite index ordering pixels by
   * their cell coordinates.
   * 2. ChargeSide index, which allows to select fired pixels with charge over
   * seed threshold for the main clustering loop.
   * 3. Cluster side index, which allows to efficiently retrieve digits
   * belonging to a cluster.
   * Note that in this setting, each cell belongs to at most one cluster. For
   * fuzzy membership, a separate cluster store has to be created.
   */

  // tags
  struct CellSide {};
  struct ChargeSide {};
  struct ClusterSide {};

  typedef boost::multi_index_container <
  ClsDigitRecord,
  boost::multi_index::indexed_by <
  boost::multi_index::ordered_unique <
  boost::multi_index::tag<CellSide>,
  boost::multi_index::composite_key <
  ClsDigitRecord,
  boost::multi_index::member <
  ClsDigitRecord, int, &ClsDigitRecord::m_uCellID
  > ,
  boost::multi_index::member <
  ClsDigitRecord, int, &ClsDigitRecord::m_vCellID
  >
  >
  > ,
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<ChargeSide>,
  boost::multi_index::member <
  ClsDigitRecord, float, &ClsDigitRecord::m_charge
  >
  > ,
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<ClusterSide>,
  boost::multi_index::member <
  ClsDigitRecord, unsigned short, &ClsDigitRecord::m_cluster
  >
  >
  >
  > ClsDigitSet;

  // CellUID-side index
  typedef ClsDigitSet::index<CellSide>::type CellSideIndex;
  typedef ClsDigitSet::index_iterator<CellSide>::type CellSideItr;

  // Seed side index
  typedef ClsDigitSet::index<ChargeSide>::type ChargeSideIndex;
  typedef ClsDigitSet::index_iterator<ChargeSide>::type ChargeSideItr;

  // Cluster-side index
  typedef ClsDigitSet::index<ClusterSide>::type ClusterSideIndex;
  typedef ClsDigitSet::index_iterator<ClusterSide>::type ClusterSideItr;

  // set of nearest neighbour cells
  typedef std::list<CellSideItr> NeighbourSet;
  typedef std::list<CellSideItr>::iterator NeighbourSetItr;


  /**
   * ClusterDigits - indexing structure used for hit reconstruction.
   *
   * This is a doubly-indexed structure based again on ClsDigitRecord,
   * but indexed differently to allow for easy computation of reconstructed
   * hit coordinates.
   */

  // tags
  struct USide {};
  struct VSide {};

  typedef boost::multi_index_container <
  ClsDigitRecord,
  boost::multi_index::indexed_by <
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<USide>,
  boost::multi_index::member <
  ClsDigitRecord, int, &ClsDigitRecord::m_uCellID
  >
  > ,
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<VSide>,
  boost::multi_index::member <
  ClsDigitRecord, int, &ClsDigitRecord::m_vCellID
  >
  >
  >
  > ClusterDigits;

  // U-side index
  typedef ClusterDigits::index<USide>::type USideIndex;
  typedef ClusterDigits::index_iterator<USide>::type USideItr;

  // V-side index
  typedef ClusterDigits::index<VSide>::type VSideIndex;
  typedef ClusterDigits::index_iterator<VSide>::type VSideItr;



} // namespace Belle2


#endif /* CLSDIGIT_H */
