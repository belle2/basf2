/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef RELATIONHOLDER_H
#define RELATIONHOLDER_H

#include <pxd/dataobjects/HitSorter.h>

#include <map>
#include <list>

// boost
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

namespace Belle2 {

//-----------------------------------------------------------------
//                 Relation manager
//-----------------------------------------------------------------

  /** AtomicRelation holds data of an atomic (one-to-one) relation.*/
  struct AtomicRelation {
    unsigned short m_from; /**< "from" index of the relation. */
    unsigned short m_to;   /**< "to" index of the relation. */
    float m_weight;
  };

  /** Index tag for "to" side access. */
  struct ToSide {};
  /** Index tag for "from" side access. */
  struct FromSide {};

  /**
   * TwoSidedRelationSet - a doubly-indexed set of atomic relations.
   *
   * The container is based on boost::multi_index_container type. It is heavily templated and
   * thus unsuitable for persistent storage of relations, but adds some efficiency to handling
   * of relations.
   * Basics of use:
   *
   * TwoSidedRelationSet mc2hits;
   * Fill the container:
   * AtomicRelation rel;
   * rel.m_from = relArray[ihit]->getFromIndex();
   * rel.m_to = relArray[ihit]->getToIndex();
   * rel.m_weight = relArray[ihit]->getWeight(); <-- not implemented!!!!
   * mc2hits.insert(rel);
   *
   * Get _references_ to "from" and "to" indices:
   * TwoSidedRelationSet::index<FromSide>::type& fromIndex = mc2hits.get<FromSide>();
   * TwoSidedRelationSet::index<ToSide>::type& toIndex = mc2hits.get<ToSide>();
   * The indices behave like std::multiset and std::set, respectively.
   * The container provides shortcuts for iterators of its indices, e.g.
   * mc2hits::index_iterator<ToSide>::type it = mc2hits.get<ToSide>().find(46);
   */

  typedef boost::multi_index_container <
  AtomicRelation,
  boost::multi_index::indexed_by <
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<FromSide>,
  boost::multi_index::member <
  AtomicRelation,
  unsigned short,
  &AtomicRelation::m_from
  >
  > ,
  boost::multi_index::ordered_non_unique <
  boost::multi_index::tag<ToSide>,
  boost::multi_index::member <
  AtomicRelation,
  unsigned short,
  &AtomicRelation::m_to
  >
  >
  >
  > TwoSidedRelationSet;

  typedef TwoSidedRelationSet::index<FromSide>::type FromSideIndex;
  typedef TwoSidedRelationSet::index<FromSide>::type::iterator FromSideItr;
  typedef TwoSidedRelationSet::index<ToSide>::type ToSideIndex;
  typedef TwoSidedRelationSet::index<ToSide>::type::iterator ToSideItr;

// for reading relations
  typedef std::list<unsigned short> RelList;
  typedef std::list<unsigned short>::iterator RelListItr;


}

#endif
