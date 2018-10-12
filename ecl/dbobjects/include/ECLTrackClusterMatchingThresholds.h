/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   * Class to hold the matching thresholds for the track-ECLCluster matching.
   */
  class ECLTrackClusterMatchingThresholds: public TObject {

  public:

    /** Default constructor */
    ECLTrackClusterMatchingThresholds() {};

    /** Constructor */
    ECLTrackClusterMatchingThresholds(const std::vector<std::pair<double, double>> matchingThresholdsFWD,
                                      const std::vector<std::pair<double, double>> matchingThresholdsBWD,
                                      const std::vector<std::pair<double, std::pair<double, double>>> matchingThresholdsBRL):
      m_matchingThresholdsFWD(matchingThresholdsFWD),
      m_matchingThresholdsBWD(matchingThresholdsBWD),
      m_matchingThresholdsBRL(matchingThresholdsBRL)
    {
    }

    /** Destructor. */
    ~ECLTrackClusterMatchingThresholds() {};

    /** Get matching thresholds for FWD */
    std::vector<std::pair<double, double>> getFWDMatchingThresholdValues() const {return m_matchingThresholdsFWD;};

    /** Get matching thresholds for BWD */
    std::vector<std::pair<double, double>> getBWDMatchingThresholdValues() const {return m_matchingThresholdsBWD;};

    /** Get matching thresholds for BRL */
    std::vector<std::pair<double, std::pair<double, double>>> getBRLMatchingThresholdValues() const {return m_matchingThresholdsBRL;};

  private:

    std::vector<std::pair<double, double>> m_matchingThresholdsFWD; /**< Matching threshold values for FWD */
    std::vector<std::pair<double, double>> m_matchingThresholdsBWD; /**< Matching threshold values for BWD */
    std::vector<std::pair<double, std::pair<double, double>>> m_matchingThresholdsBRL; /**< Matching threshold values for BRL */

    ClassDef(ECLTrackClusterMatchingThresholds, 1); /**< ClassDef */
  };
} // namespace Belle2
