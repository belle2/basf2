/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Oliver Frost                                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/WithWeight.h>
#include <tracking/trackFindingCDC/utilities/CallIfApplicable.h>

#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// A mixin class to attach a set of weighted items to a class
    template<class T, class AItem>
    class WithWeightedItems : public T {

    private:
      /// Type of the base class
      typedef T Super;

      /// Type of this class
      typedef WithWeightedItems<T, AItem> This;

    public:
      /// Make the constructor of the base type available
      using T::T;

      /// Allow assignment of the base class.
      using T::operator=;

      /// Also forward the copy constructor form the base class object.
      explicit WithWeightedItems(const T& t): T(t) {}

      /// Cumulated weight of the contained items.
      Weight getWeight() const
      {
        return std::accumulate(m_items.begin(), m_items.end(), static_cast<Weight>(0.0),
        [](Weight accumulatedWeight, const WithWeight<AItem>& weightedItem) {
          return accumulatedWeight + weightedItem.getWeight();
        });
      }

      /// Erase items from this node that satisfy the predicate.
      template<class APredicate>
      void eraseIf(APredicate& predicate)
      {
        auto itFirstToRemove = std::remove_if(m_items.begin(), m_items.end(), predicate);
        m_items.erase(itFirstToRemove, m_items.end());
      }

      /// Add an item with weight.
      void insert(const AItem& item, Weight weight = 1.0)
      {
        m_items.push_back(WithWeight<AItem>(item));
        m_items.back().setWeight(weight);
      }

      /// Add the items from another item range assigning a weight from the predicate.
      template<class AMeasure>
      void insert(const This& items,
                  AMeasure& measure)
      {
        for (AItem item : items) {
          const Weight weight = measure(item);
          if (not std::isnan(weight)) {
            insert(item, weight);
          }
        }
      }

      /// Begin iterator of the contained items.
      typename std::vector< WithWeight<AItem> >::iterator
      begin()
      { return m_items.begin(); }

      /// Begin iterator of the contained items.
      typename std::vector< WithWeight<AItem> >::const_iterator
      begin() const
      { return m_items.begin(); }

      /// End iterator of the contained items.
      typename std::vector< WithWeight<AItem> >::iterator
      end()
      { return m_items.end(); }

      /// End iterator of the contained items.
      typename std::vector< WithWeight<AItem> >::const_iterator
      end() const
      { return m_items.end(); }

      /// Getter for the number of items
      size_t size() const
      { return m_items.size(); }

    public:
      /// Clear the contained items.
      void clear()
      {
        m_items.clear();
        Super& super = *this;
        clearIfApplicable(super);
      }

    private:
      /// Memory for the weighted items.
      std::vector< WithWeight<AItem> > m_items;
    };
  }
}
