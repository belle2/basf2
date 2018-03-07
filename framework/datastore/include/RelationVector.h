/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationEntry.h>
#include <framework/utilities/ArrayIterator.h>

#include <vector>
#include <string>
#include <functional>

namespace Belle2 {
  /** base class for RelationVector<T> */
  class RelationVectorBase {
  public:
    /** Construct empty set. */
    RelationVectorBase() { }

    /** Constructor.  */
    RelationVectorBase(const std::string& name, int index, const std::vector<Belle2::RelationEntry>& relations,
                       const std::vector<std::string>& names): m_name(name), m_index(index), m_relations(relations), m_relationNames(names) {}

  protected:
    /** add another list of relations. (internal use) */
    void add(const RelationVectorBase& other);

    /** apply function to the relation associated with the RelationEntry at given index. */
    void apply(int index, const std::function<void(std::vector<unsigned int>&, std::vector<float>&, size_t)>& f);

    std::string m_name; /**< entry name of array containing object these relations belong to. */
    int m_index{ -1}; /**< index of object these relations belong to. */
    std::vector<RelationEntry> m_relations;  /**< The vector of relation entries */
    std::vector<std::string> m_relationNames;  /**< Names of associated relations. */
    friend class DataStore;
  };

  /** Class for type safe access to objects that are referred to in relations.
   *
   * Objects of this class are returned by RelationsObject functinons to provide
   * type safe access to the objects in a vector of relations returned by
   * the data store.
   *
   * Besides accessing objects/weights directly using operator[](int) and weight(int),
   * you can also iterate over the objects directly:
      \code
      const MCParticle* particle = particles[i];
      for (const CDCSimHit& simhit :  particle->getRelationsTo<CDCSimHit>()) {
        //do things with simhit
      }
      \endcode
   * If you want to modify the related objects in the loop body, you can use a
   * non-const reference instead.
   *
   * Use setWeight() to modify the weight of a single relation.
   * Use remove() to delete a relation.
   */
  template <class T> class RelationVector : protected RelationVectorBase {
  public:
    /** STL-like iterator over the T objects (not T* ). */
    typedef ArrayIterator<RelationVector<T>, T> iterator;
    /** STL-like const_iterator over the T objects (not T* ). */
    typedef ArrayIterator<RelationVector<T>, const T> const_iterator;


    /** Constructor.
     *
     *  @param b Not type-safe base class containing the data
     */
    explicit RelationVector(const RelationVectorBase& b): RelationVectorBase(b) {}

    /** Accessor for the relations vector.
     *
     *  @return           Vector of RelationEntry objects.
     */
    const std::vector<Belle2::RelationEntry>& relations() const {return m_relations;}

    /** Get number of relations.  */
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
    float weight(int index) const {return m_relations[index].weight;}

    /** Remove relation at given index.
     *
     * This will decrease size() by one. Iterators pointing beyond given
     * index will be invalidated when calling this function.
     *
     * @note May be slow if done frequently.
     */
    void remove(int index)
    {
      apply(index, [](std::vector<unsigned int>& indices, std::vector<float>& weights,
      size_t elidx) {
        indices.erase(indices.begin() + elidx);
        weights.erase(weights.begin() + elidx);
      });

      m_relations.erase(m_relations.begin() + index);
    }

    /** Set a new weight for the given relation. */
    void setWeight(int index, float weight)
    {
      apply(index, [weight](std::vector<unsigned int>&, std::vector<float>& weights, size_t elidx) {
        weights[elidx] = weight;
      });

      m_relations[index].weight = weight;
    }

    /** Return iterator to first entry. */
    iterator begin() { return iterator(this, 0); }
    /** Return iterator to last entry +1. */
    iterator end() { return iterator(this, size()); }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { return const_iterator(this, 0); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { return const_iterator(this, size()); }
  };
}
