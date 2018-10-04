/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    bool add(TItem const& item, TEstimator est,
             EstimatorComparison estComparison
    = [](TEstimator currentBest, TEstimator newEst) {return newEst < currentBest;}
            )
    {
      if (!m_oneMatch) {
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
      return m_oneMatch;
    }

    /**
     * @return Returns a reference to the candidate with the best match
     * according to the estimator value.
     */
    TItem const& getBestMatch() const
    {
      return m_bestMatch;
    }

  private:

    /**
     * Set a new item as the best match
     */
    void setBestMatch(TItem const& item, TEstimator est)
    {
      m_bestMatch = item;
      m_bestEstimator = est;
      m_oneMatch = true;
    }

    /// Stores the best matched item
    TItem m_bestMatch;

    // stores if at least match item has been set
    bool m_oneMatch = false;

    /// Stores the estimator value of the best match
    TEstimator m_bestEstimator = TEstimator();
  };

} //Belle2
