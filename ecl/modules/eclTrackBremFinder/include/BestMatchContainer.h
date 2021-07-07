/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <functional>

namespace Belle2 {

  /**
   * Multiple entries can be added, but only the one will be kept, which
   * has the best quality estimator.
   * This class can be used in cases where the best-match candidate is searched
   */

  template <class TItem, class TEstimator >
  class BestMatchContainer {
  public:

    /**
     * Lambda typedef for the function comparing estimators. Returns true if
     * the second parameter is a better estimation than the first one.
     */
    typedef std::function< bool(TEstimator, TEstimator)> EstimatorComparison;

    /**
     * Add a new item with an estimator value
     *
     * @param estComparison The default estimator will favor items with
     *  smaller estimates
     */
    bool add(TItem item, TEstimator est,
             EstimatorComparison estComparison
    = [](TEstimator currentBest, TEstimator newEst) {return newEst < currentBest;}
            )
    {
      if (!hasMatch()) {
        // not best match yet, take this one !
        setBestMatch(item , est);
        return true;
      }

      if (estComparison(m_bestEstimator, est)) {
        setBestMatch(item, est);
        return true;
      }

      // best match was not updated
      return false;
    }

    /**
     * @return Returns true if at least one candidate has been matched.
     */
    bool hasMatch() const
    {
      return static_cast<bool>(m_bestMatch);
    }

    /**
     * @return Returns a reference to the candidate with the best match
     * according to the estimator value.
     */
    TItem const& getBestMatch() const
    {
      return *m_bestMatch;
    }

  private:

    /**
     * Set a new item as the best match
     */
    void setBestMatch(TItem item, TEstimator est)
    {
      m_bestMatch = item;
      m_bestEstimator = est;
    }

    /// Stores the best matched item
    std::optional<TItem> m_bestMatch;

    /// Stores the estimator value of the best match
    TEstimator m_bestEstimator = TEstimator();
  };

} //Belle2
