/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <vector>
#include <utility>

namespace Belle2 {
  /** Class to store a single element of a relation.
   *
   *  Supports 1:n relations with individual weights.
   */
  class RelationElement: public TObject {
  public:

    /** type used for indices. */
    typedef unsigned int index_type;

    /** type used for weights. */
    typedef float weight_type;

    /** Empty constructor for ROOT */
    RelationElement();


    /** Constructor for a 1:1 relation.
     *
     *  @param from    index to point from
     *  @param to      index to point to
     *  @param weight  weight of the relation
     */
    RelationElement(index_type from, index_type to, weight_type weight = 1.0):
      TObject(), m_from(from), m_to(1, to), m_weights(1, weight) {}

    /** Constructor for a 1:n relation.
     *
     *  @param from    index to point from
     *  @param to      indices to point to
     *  @param weights weights of the relation
     */
    RelationElement(index_type from, const std::vector<index_type>& to, const std::vector<weight_type>& weights);

    /** Constructor for a 1:n relation.
     *
     *  @param from    index to point from
     *  @param begin   iterator pointing to the begin of a sequence of
     *                 std::pair<index_type,weight_type> or compatible
     *  @param end     iterator pointing to the end of a sequence of
     *                 std::pair<index_type,weight_type> or compatible
     */
    template <class InputIterator> RelationElement(index_type from, const InputIterator& begin, const InputIterator& end):
      TObject(), m_from(from)
    {
      setToIndices(begin, end);
    }

    /** Get index we point from. */
    index_type                        getFromIndex()         const { return m_from; }

    /** Get number of indices we points to. */
    size_t                            getSize()              const { return m_to.size(); }

    /** Get nth pair of index,weight we point to. */
    std::pair<index_type, weight_type> getTo(size_t n = 0)      const { return std::make_pair(m_to[n], m_weights[n]); }

    /** Get nth index we point to. */
    index_type                        getToIndex(size_t n = 0) const { return m_to[n]; }

    /** Get nth weight we point to. */
    weight_type                       getWeight(size_t n = 0)  const { return m_weights[n]; }

    /** Get vector of indices we point to. */
    const std::vector<index_type>&     getToIndices()         const { return m_to; }

    /** Get vector of weights we point to. */
    const std::vector<weight_type>&    getWeights()           const { return m_weights; }

    /** Set index we point from. */
    void setFromIndex(index_type from) { m_from = from; }

    /** Set index we point to, converts relation to 1:1 and discards all existing to-indices. */
    void setToIndex(index_type to, weight_type weight = 1.0)
    {
      m_to = std::vector<index_type>(1, to);
      m_weights = std::vector<weight_type>(1, weight);
    }

    /** Set new indices and weights we point to
     *
     *  @param begin   iterator pointing to the begin of a sequence of
     *                 std::pair<index_type,weight_type> or compatible
     *  @param end     iterator pointing to the end of a sequence of
     *                 std::pair<index_type,weight_type> or compatible
     */
    template<class InputIterator> void setToIndices(InputIterator begin, const InputIterator& end)
    {
      for (; begin != end; ++begin) {
        m_to.push_back(begin->first);
        m_weights.push_back(begin->second);
      }
    }

  protected:

    /** index we point from. */
    index_type m_from;

    /** indices we point to. */
    std::vector<index_type> m_to;

    /** weights for to-indices. */
    std::vector<weight_type> m_weights;

    ClassDef(RelationElement, 1); /**< Class to store a single element of a relation. */
  };

}
