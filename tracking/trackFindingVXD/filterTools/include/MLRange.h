/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TTree.h>
#include <array>

namespace Belle2 {

  /**
   * Range used for the Machine Learning assisted TrackFinding approach.
   *
   * NOTE: This is where all the magic happens! For other Filters the SelectionVariable is responsible for calculating a value
   * and then passes the calculated value to a Range, where a simple comparison occurs. In the case of ML Filters this is not
   * easily possible if there should be different ML classifiers for different detector regions. Thus, the SelectionVariable
   * simply passes down the hits and this Range is responsible for the calculation and the comparison.
   */
  template<typename ClassifierType, size_t Ndims = 9, typename CutType = double>
  class MLRange {
    /// Pointer to classifier to be used
    ClassifierType* m_classifier;
    /// Cut value
    CutType m_cut;

  public:
    /** Constructor
     * @param classifier : classifier to be used
     * @param cut : cut value
     */
    MLRange(ClassifierType* classifier, CutType cut) : m_classifier(classifier), m_cut(cut) { ; }

    /**
     * method used to decide if a hit combination passes the Machine Learning filter
     *
     * @param hits is the three hit combination (for the moment, maybe this will get templated in the future) that gets
     * passed down from the MLHandover selection variable.
     */
    inline bool contains(std::array<double, Ndims> hits) const { return !(m_classifier->analyze(hits) < m_cut); }

    /**
     * For Storing this range
     *
     * TODO
     */
    void persist(TTree* /*t*/, const std::string& /*branchname*/, const std::string& /*variablename*/)
    {
      // TODO
    }
  };
}
