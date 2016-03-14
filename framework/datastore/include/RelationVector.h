/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONVECTOR_H
#define RELATIONVECTOR_H

#include <framework/datastore/RelationEntry.h>
#include <framework/utilities/ArrayIterator.h>

#include <vector>


namespace Belle2 {
  /** Class for type safe access to objects that are referred to in relations.
   *
   *  This class is supposed to be used by the RelationsInterface to provide
   *  type safe access to the objects in a vector of relations returned by
   *  the data store.
   *
   * Besides accessing objects/weights directly using operator[](int) and weight(int),
   * you can also iterate over the objects directly:
      \code
      const MCParticle* particle = particles[i];
      for (const CDCSimHit& simhit :  particle->getRelationsTo<CDCSimHit>()) {
        //do things with simhit
      }
      \endcode
   * If you want to modify the related objects, you can use a non-const reference instead.
   */
  template <class T> class RelationVector {
  public:
    /** STL-like iterator over the T objects (not T* ). */
    typedef ArrayIterator<RelationVector<T>, T> iterator;
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ArrayIterator<RelationVector<T>, const T> const_iterator;


    /** Constructor.
     *
     *  @param relations  The vector of relation objects.
     */
    explicit RelationVector(const std::vector<Belle2::RelationEntry>& relations): m_relations(relations) {}

    /** Accessor for the relations vector.
     *
     *  @return           Vector of RelationEntry objects.
     */
    const std::vector<Belle2::RelationEntry>& relations() const {return m_relations;}


    /** Get number of relations.
     *
     *  @return           Number of relations.
     */
    size_t size() const { return m_relations.size();}

    /** Get object with index.
     *
     *  @param index      Index of relation.
     *  @return           Object that the relation points to.
     */
    T*     object(int index) const {return static_cast<T*>(m_relations[index].object);}

    /** Get object with index.
     *
     *  @param index      Index of relation.
     *  @return           Object that the relation points to.
     */
    T*     operator[](int index) const {return object(index);}

    /** Get weight with index.
     *
     *  @param index      Index of relation.
     *  @return           Weight that the relation has.
     */
    double weight(int index) const {return m_relations[index].weight;}


    /** Return iterator to first entry. */
    iterator begin() { return iterator(this, 0); }
    /** Return iterator to last entry +1. */
    iterator end() { return iterator(this, size()); }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { return const_iterator(this, 0); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { return const_iterator(this, size()); }

  private:
    std::vector<Belle2::RelationEntry> m_relations;  /**< The vector of relation entries */
  };
}
#endif
