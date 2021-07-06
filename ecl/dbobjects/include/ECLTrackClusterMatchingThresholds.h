/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    ECLTrackClusterMatchingThresholds(const std::vector<std::pair<double, double>>& matchingThresholdsFWD,
                                      const std::vector<std::pair<double, double>>& matchingThresholdsBWD,
                                      const std::vector<std::pair<double, std::pair<double, double>>>& matchingThresholdsBRL):
      m_matchingThresholdsFWD(matchingThresholdsFWD),
      m_matchingThresholdsBWD(matchingThresholdsBWD),
      m_matchingThresholdsBRL(matchingThresholdsBRL)
    {
    }

    /** Destructor. */
    ~ECLTrackClusterMatchingThresholds() {};

    /** Get matching thresholds for FWD */
    const std::vector<std::pair<double, double>>& getFWDMatchingThresholdValues() const {return m_matchingThresholdsFWD;};

    /** Get matching thresholds for BWD */
    const std::vector<std::pair<double, double>>& getBWDMatchingThresholdValues() const {return m_matchingThresholdsBWD;};

    /** Get matching thresholds for BRL */
    const std::vector<std::pair<double, std::pair<double, double>>>& getBRLMatchingThresholdValues() const {return m_matchingThresholdsBRL;};

  private:

    std::vector<std::pair<double, double>> m_matchingThresholdsFWD; /**< Matching threshold values for FWD */
    std::vector<std::pair<double, double>> m_matchingThresholdsBWD; /**< Matching threshold values for BWD */
    std::vector<std::pair<double, std::pair<double, double>>> m_matchingThresholdsBRL; /**< Matching threshold values for BRL */

    ClassDef(ECLTrackClusterMatchingThresholds, 1); /**< ClassDef */
  };
} // namespace Belle2
