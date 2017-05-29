/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>
#include <map>
#include <vector>
#include <tuple>
#include <utility>

namespace Belle2 {

  /**
   * Class to store TOP log likelihoods (output of TOPReconstructor).
   * relation from Tracks
   * filled in top/modules/TOPReconstruction/src/TOPReconstructorModule.cc
   */

  class TOPSmallestPullCollection : public RelationsObject {

  public:
    /**
     * default constructor
     */
    TOPSmallestPullCollection() { }
    /**
     * saves the smallest pull
     */
    void setSmallestPulls(const std::vector<float> pulls, const std::vector<int> bestpeaks = {})
    {
      m_smallest_pulls = pulls;
      m_ks1 = bestpeaks; // optional
    }
    void setPullsFromSmallestDiff(const std::vector<float> pulls, const std::vector<int> bestpeaks = {})
    {
      m_pulls_from_smallest = pulls;
      m_ks2 = bestpeaks; // optional
    }
    std::vector<float> m_smallest_pulls; /**< the smallest pulls */
    std::vector<int> m_ks1; /**< the index of the peak giving the smallest pull */
    std::vector<float> m_pulls_from_smallest; /**< the pulls from the closest peak */
    std::vector<int> m_ks2; /**< the index of the closest pull */

  private:
    ClassDef(TOPSmallestPullCollection, 1); /**< ClassDef */
  };
} // end namespace Belle2

