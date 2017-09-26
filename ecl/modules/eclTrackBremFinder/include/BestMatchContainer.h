/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BESTMATCHCONTAINER_H
#define BESTMATCHCONTAINER_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLShower.h>

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

    typedef std::function< bool(TEstimator, TEstimator)> EstimatorComparison;

    /*  static constexpr EstimatorComparison MatchSmaller = [](TEstimator currentBest, TEstimator newEst) {return newEst < currentBest;};
      static EstimatorComparison MatchLarger = [](TEstimator currentBest, TEstimator newEst) {return newEst > currentBest;};
    */
    bool add(TItem const& item, TEstimator est,
             // the default estimator will match via smaller estimators
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

    bool hasMatch() const
    {
      return m_oneMatch;
    }

    TItem const& getBestMatch() const
    {
      return m_bestMatch;
    }

  private:
    void setBestMatch(TItem const& item, TEstimator est)
    {
      m_bestMatch = item;
      m_bestEstimator = est;
      m_oneMatch = true;
    }

    TItem m_bestMatch;
    TEstimator m_bestEstimator = TEstimator();
    bool m_oneMatch = false;

  };

} //Belle2
#endif
