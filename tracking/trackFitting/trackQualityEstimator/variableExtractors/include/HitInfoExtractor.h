/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sebastian Racs                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/utilities/Named.h>
#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <numeric>


namespace Belle2 {
  /// class to extract info from individual clusters and combine for SPTC
  class HitInfoExtractor : public VariableExtractor {
  public:
    /** Constructor fills variableSet with variables to be extracted
     * @param variableSet set of variable to be filled
     * @param useTimingInfo whether to use the timing info in clusters
     */
    HitInfoExtractor(std::vector<Named<float*>>& variableSet) :
      VariableExtractor()
    {
      addVariable("N_TP_noKalmanFitterInfo", variableSet);

      initializeStats("weight", variableSet);
      initializeStats("smoothedChi2", variableSet);
    }

    /// extract variables from SpacePoints
    void extractVariables(const RecoTrack& recoTrack)
    {
      //get fitting parameters
      const std::vector<RecoHitInformation*>& relatedRecoHitInformation = recoTrack.getRecoHitInformations();
      std::vector<const genfit::KalmanFitterInfo*> kalmanFitterInfos;
      kalmanFitterInfos.reserve(relatedRecoHitInformation.size());

      int n_trackPoint_noKalmanFitterInfo = 0;

      for (const RecoHitInformation* recoHitInformation : relatedRecoHitInformation) {
        const genfit::TrackPoint* trackPoint = recoTrack.getCreatedTrackPoint(recoHitInformation);
        if (trackPoint) {
          const genfit::KalmanFitterInfo* kalmanFitterInfo = trackPoint->getKalmanFitterInfo();
          if (kalmanFitterInfo) {
            kalmanFitterInfos.push_back(kalmanFitterInfo);
          } else {
            n_trackPoint_noKalmanFitterInfo++;
          }
        }
      }
      m_variables.at("N_TP_noKalmanFitterInfo") = n_trackPoint_noKalmanFitterInfo;

      std::vector<float> values(kalmanFitterInfos.size());

      for (unsigned int i = 0; i < kalmanFitterInfos.size(); ++i) {
        values[i] = kalmanFitterInfos[i]->getWeights().front();
      }
      setStats("weight", values);

      for (unsigned int i = 0; i < kalmanFitterInfos.size(); ++i) {
        try {
          values[i] = kalmanFitterInfos[i]->getSmoothedChi2(); //produces error for event 98
        } catch (const std::exception& e) {
          B2WARNING("HitInfoExtractor: Caught exception in kalmanFitterInfos[i]->getSmoothedChi2() \n"
                    << "-->" << e.what());
          values[i] = NAN;
        }
      }
      setStats("smoothedChi2", values);

    }

  protected:
    /// initialize statistics subsets of variables from clusters that get combined for SPTC
    void initializeStats(std::string identifier, std::vector<Named<float*>>& variables)
    {
      addVariable(identifier + "_max", variables);
      addVariable(identifier + "_min", variables);
      addVariable(identifier + "_mean", variables);
      addVariable(identifier + "_std", variables);
      addVariable(identifier + "_median", variables);
//      addVariable(identifier + "_n_zeros", variables);
    }

    /// calculated statistics and saves them in variable set
    void setStats(std::string identifier, std::vector<float>& values)
    {

      int size = values.size();
      if (values.size() == 0) {
        m_variables.at(identifier + "_max") = NAN;
        m_variables.at(identifier + "_min") = NAN;
        m_variables.at(identifier + "_mean") = NAN;
        m_variables.at(identifier + "_std") = NAN;
        m_variables.at(identifier + "_median") = NAN;
//        m_variables.at(identifier + "_n_zeros") = NAN;
        return;
      }

      std::sort(values.begin(), values.end());

      float sum = std::accumulate(values.begin(), values.end(), 0.0);
      float mean = sum / size;
      m_variables.at(identifier + "_mean") = mean;

      float variance = 0.; //int n_zeros = 0;
      for (float value : values) {
        variance += (value - mean) * (value - mean);
//        if (value == 0)
//          n_zeros++;
      }
      // number of 0 values
//      m_variables.at(identifier + "_n_zeros") = n_zeros;
      // variance and standard deviation
      variance /= size - 1;
      float stddev = std::sqrt(variance);
      m_variables.at(identifier + "_std") = stddev;

      //min and max
      m_variables.at(identifier + "_min") = values.front();
      m_variables.at(identifier + "_max") = values.back();

      //median
      float median = size % 2 ? values[size / 2] : 0.5 * (values[size / 2] + values[size / 2 - 1]);
      m_variables.at(identifier + "_median") = median;
    }

  };
}
