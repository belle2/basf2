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

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// A mixin class to attach a set of weighted items to a class
    template <class T, class AItem>
    class WithWeightedItems : public T {

    private:
      /// Type of the base class
      using Super = T;

      /// Type of this class
      using This = WithWeightedItems<T, AItem>;

    public:
      /// Make the constructor of the base type available
      using T::T;

      /// Allow assignment of the base class.
      using T::operator=;

      /// Also forward the copy constructor form the base class object.
      explicit WithWeightedItems(const T& t)
        : T(t)
      {
      }

      /// Cumulated weight of the contained items.
      Weight getWeight() const
      {
        return std::accumulate(begin(),
                               end(),
                               static_cast<Weight>(0.0),
        [](Weight accumulatedWeight, const WithWeight<AItem>& weightedItem) {
          return accumulatedWeight + weightedItem.getWeight();
        });
      }

      /// Erase items from this node that satisfy the predicate.
      template <class APredicate>
      void eraseIf(const APredicate& predicate)
      {
        // Sneakily hide the items instead of erasing them.
        // auto notPredicate = [&predicate](AItem & item) { return not predicate(item); };
        // m_itEnd = std::partition(m_items.begin(), m_items.end(), notPredicate);
        // Properly delete use the following.
        m_itEnd = std::remove_if(m_items.begin(), m_items.end(), predicate);
        m_items.erase(m_itEnd, m_items.end());
      }

      /// Add an item with weight.
      void insert(const AItem& item, Weight weight = 1.0)
      {
        m_itEnd = m_items.insert(end(), WithWeight<AItem>(item));
        m_itEnd->setWeight(weight);
        ++m_itEnd;
      }

      /// Add the items from another item range assigning a weight from the predicate.
      template <class AMeasure>
      void insert(const This& items, AMeasure& measure)
      {
        for (AItem item : items.m_items) {
          const Weight weight = measure(item);
          if (not std::isnan(weight)) {
            insert(item, weight);
          }
        }
      }

      /// Begin iterator of the contained items.
      typename std::vector<WithWeight<AItem>>::iterator begin()
      {
        return m_items.begin();
      }

      /// Begin iterator of the contained items.
      typename std::vector<WithWeight<AItem>>::const_iterator begin() const
      {
        return m_items.begin();
      }

      /// End iterator of the contained items.
      typename std::vector<WithWeight<AItem>>::iterator end()
      {
        return m_itEnd;
      }

      /// End iterator of the contained items.
      typename std::vector<WithWeight<AItem>>::const_iterator end() const
      {
        return m_itEnd;
      }

      /// Getter for the number of items
      size_t size() const
      {
        return std::distance(begin(), end());
      }

    public:
      /// Clear the contained items.
      void clear()
      {
        m_items.clear();
        m_itEnd = m_items.end();
        Super& super = *this;
        clearIfApplicable(super);
      }

    private:
      /// Memory for the weighted items.
      std::vector<WithWeight<AItem>> m_items;

      /// Memory for the end of the items that are not erased.
      typename std::vector<WithWeight<AItem>>::iterator m_itEnd{m_items.end()};
    };
  }
}
