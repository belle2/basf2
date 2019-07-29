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
#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <genfit/KalmanFitterInfo.h>
#include <genfit/TrackPoint.h>
#include <numeric>
#include <algorithm>
#include <optional>

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
      addVariable("N_TrackPoints_without_KalmanFitterInfo", variableSet);
      addVariable("N_Hits_without_TrackPoint", variableSet);

      initializeStats("weight", variableSet);
      initializeStats("smoothedChi2", variableSet);
    }

    /// extract variables from SpacePoints
    void extractVariables(const RecoTrack& recoTrack)
    {
      // get fitting parameters
      const std::vector<RecoHitInformation*>& recoHitInformations = recoTrack.getRecoHitInformations(true);
      std::vector<const genfit::KalmanFitterInfo*> kalmanFitterInfos;
      kalmanFitterInfos.reserve(recoHitInformations.size());

      int n_no_trackPoint = 0;  // number of recoHitInformations without related track points
      int n_no_KalmanFitterInfo = 0;  // number of track points without related KalmanFitterInfo
      std::optional<float> weight_firstCDCHit;
      std::optional<float> weight_lastSVDHit;
      std::optional<float> smoothedChi2_firstCDCHit;
      std::optional<float> smoothedChi2_lastSVDHit;
      for (const RecoHitInformation* recoHitInformation : recoHitInformations) {
        const genfit::TrackPoint* trackPoint = recoTrack.getCreatedTrackPoint(recoHitInformation);
        if (trackPoint) {
          const genfit::KalmanFitterInfo* kalmanFitterInfo = trackPoint->getKalmanFitterInfo();
          if (kalmanFitterInfo) {
            kalmanFitterInfos.push_back(kalmanFitterInfo);
            if ((not weight_firstCDCHit)
                and (recoHitInformation->getTrackingDetector() == recoHitInformation->c_CDC)) {
              weight_firstCDCHit = kalmanFitterInfo->getWeights().front();
              smoothedChi2_firstCDCHit = this->getSmoothedChi2(kalmanFitterInfo);
            }
            if (recoHitInformation->getTrackingDetector() == recoHitInformation->c_SVD) {
              weight_lastSVDHit = kalmanFitterInfo->getWeights().front();
              smoothedChi2_lastSVDHit = this->getSmoothedChi2(kalmanFitterInfo);
            }
          } else {
            n_no_KalmanFitterInfo++;
          }
        } else {
          n_no_trackPoint++;
        }
      }
      m_variables.at("N_Hits_without_TrackPoint") = n_no_trackPoint;
      m_variables.at("N_TrackPoints_without_KalmanFitterInfo") = n_no_KalmanFitterInfo;
      m_variables.at("weight_lastSVDHit") = weight_lastSVDHit.value_or(-1);
      m_variables.at("weight_firstCDCHit") = weight_firstCDCHit.value_or(-1);

      std::vector<float> fitWeights;
      std::vector<float> chi2Values;
      fitWeights.reserve(kalmanFitterInfos.size());
      chi2Values.reserve(kalmanFitterInfos.size());
      for (const auto& kalmanFitterInfo : kalmanFitterInfos) {
        fitWeights.push_back(kalmanFitterInfo->getWeights().front());
        chi2Values.push_back(this->getSmoothedChi2(kalmanFitterInfo).value_or(-1.0));
      }
      setStats("weight", fitWeights);
      setStats("smoothedChi2", chi2Values);
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
      addVariable(identifier + "_n_zeros", variables);
      addVariable(identifier + "_firstCDChit", variables);
      addVariable(identifier + "_lastSVDhit", variables);
    }

    /// calculated statistics and saves them in variable set
    void setStats
    (std::string identifier, std::vector<float>& values)
    {
      int size = values.size();
      if (values.size() == 0) {
        m_variables.at(identifier + "_max") =  -1.;
        m_variables.at(identifier + "_min") =  -1.;
        m_variables.at(identifier + "_mean") =  -1.;
        m_variables.at(identifier + "_std") =  -1.;
        m_variables.at(identifier + "_median") =  -1.;
        m_variables.at(identifier + "_n_zeros") = -1;
        m_variables.at(identifier + "_firstCDChit") = -1;
        m_variables.at(identifier + "_lastSVDhit") = -1;
        return;
      }

      std::sort(values.begin(), values.end());

      float sum = std::accumulate(values.begin(), values.end(), 0.0);
      float mean = sum / size;
      m_variables.at(identifier + "_mean") = mean;
      float variance = 0.;
      int n_zeros = 0;
      for (float value : values) {
        variance += (value - mean) * (value - mean);
        if (value == 0)
          n_zeros++;
      }
      m_variables.at(identifier + "_n_zeros") = n_zeros;
      variance /= size - 1;
      float stddev = std::sqrt(variance);
      m_variables.at(identifier + "_std") = stddev;
      m_variables.at(identifier + "_min") = values.front();
      m_variables.at(identifier + "_max") = values.back();
      float median = size % 2 ? values[size / 2] : 0.5 * (values[size / 2] + values[size / 2 - 1]);
      m_variables.at(identifier + "_median") = median;
    }

  private:
    /// Helper function to safely get Chi2 from a KalmanFitterInfo object if available, and if not return nullopt
    std::optional<float> getSmoothedChi2(const genfit::KalmanFitterInfo* kalmanFitterInfo)
    {
      try {
        return kalmanFitterInfo->getSmoothedChi2();
      } catch (const std::exception& e) {
        B2WARNING("HitInfoExtractor: Caught exception in kalmanFitterInfos[i]->getSmoothedChi2() \n"
                  << "-->" << e.what());
        return std::nullopt;
      }
    }
  };
}
