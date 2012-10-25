/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef GETRELATED
#define GETRELATED

#include <framework/datastore/RelationIndex.h>

// --------------------
// under developement !
// --------------------

namespace Belle2 {

  /* get the first related element
   * user does not need to know in which way the relation is defined
   * @param u element pointer (NULL also allowed)
   * @return related element pointer or NULL for no relation
   */
  template <class T, class U>
  const T* getRelated(const U* u)
  {

    if (!u) return 0;

    RelationIndex<T, U> t2u;
    if (t2u) {
      if (t2u.getFirstElementTo(u)) return t2u.getFirstElementTo(u)->from;
      return 0;
    }
    RelationIndex<U, T> u2t;
    if (u2t) {
      if (u2t.getFirstElementFrom(u)) return u2t.getFirstElementFrom(u)->to;
      return 0;
    }
    return 0;
  }


  /* get a vector of related elements
   * user does not need to know in which way the relation is defined
   * @param u element pointer (NULL also allowed)
   * @return vector of related element pointers (empty for no relation)
   */
  template <class T, class U>
  std::vector<const T*> getRelatedVector(const U* u)
  {

    std::vector<const T*> vec;
    if (!u) return vec;

    RelationIndex<T, U> t2u;
    if (t2u) {
      typedef typename RelationIndex<T, U>::Element relElement_t;
      BOOST_FOREACH(const relElement_t & rel, t2u.getElementsTo(u)) {
        const T* t = rel.from;
        vec.push_back(t);
      }
      return vec;
    }
    RelationIndex<U, T> u2t;
    if (u2t) {
      typedef typename RelationIndex<U, T>::Element relElement_t;
      BOOST_FOREACH(const relElement_t & rel, u2t.getElementsFrom(u)) {
        const T* t = rel.to;
        vec.push_back(t);
      }
      return vec;
    }
    return vec;
  }


  /* get a vector of pairs of related elements and weights
   * user does not need to know in which way the relation is defined
   * @param u element pointer (NULL also allowed)
   * @return vector of related element pointers and weights (empty for no relation)
   */
  template <class T, class U>
  std::vector< std::pair<const T*, double> > getRelatedPairs(const U* u)
  {

    std::vector< std::pair<const T*, double> > vec;
    if (!u) return vec;

    RelationIndex<T, U> t2u;
    if (t2u) {
      typedef typename RelationIndex<T, U>::Element relElement_t;
      BOOST_FOREACH(const relElement_t & rel, t2u.getElementsTo(u)) {
        const T* t = rel.from;
        double w = rel.weight;
        std::pair<const T*, double> p(t, w);
        vec.push_back(p);
      }
      return vec;
    }
    RelationIndex<U, T> u2t;
    if (u2t) {
      typedef typename RelationIndex<U, T>::Element relElement_t;
      BOOST_FOREACH(const relElement_t & rel, u2t.getElementsFrom(u)) {
        const T* t = rel.to;
        double w = rel.weight;
        std::pair<const T*, double> p(t, w);
        vec.push_back(p);
      }
      return vec;
    }
    return vec;
  }


} // Belle2 namespace

#endif



