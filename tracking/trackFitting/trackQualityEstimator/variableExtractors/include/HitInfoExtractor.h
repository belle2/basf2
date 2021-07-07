/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
    explicit HitInfoExtractor(std::vector<Named<float*>>& variableSet) :
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
      for (const RecoHitInformation* recoHitInformation : recoHitInformations) {
        const genfit::TrackPoint* trackPoint = recoTrack.getCreatedTrackPoint(recoHitInformation);
        if (trackPoint) {
          const genfit::KalmanFitterInfo* kalmanFitterInfo = trackPoint->getKalmanFitterInfo();
          if (kalmanFitterInfo) {
            kalmanFitterInfos.push_back(kalmanFitterInfo);
          } else {
            n_no_KalmanFitterInfo++;
          }
        } else {
          n_no_trackPoint++;
        }
      }
      m_variables.at("N_Hits_without_TrackPoint") = n_no_trackPoint;
      m_variables.at("N_TrackPoints_without_KalmanFitterInfo") = n_no_KalmanFitterInfo;

      // define lambda to which checks if hitinfo has kalman info and is from a specific detector
      // use that via std::find_if to find first/last fitted hits from that detector
      const auto hitHasKalmanInfoAndIsFromDetector =
        [&recoTrack, this](const auto & recoHitInformation,
      RecoHitInformation::RecoHitDetector trackingDetector) {
        return (this->getKalmanFitterInfo(recoTrack, recoHitInformation)
                and (recoHitInformation->getTrackingDetector() == trackingDetector));
      };
      const auto hitHasKalmanInfoAndIsFromCDC = std::bind(hitHasKalmanInfoAndIsFromDetector,
                                                          std::placeholders::_1,
                                                          RecoHitInformation::c_CDC);
      const auto hitHasKalmanInfoAndIsFromSVD = std::bind(hitHasKalmanInfoAndIsFromDetector,
                                                          std::placeholders::_1,
                                                          RecoHitInformation::c_SVD);

      // find first CDC hit with Kalman info and extract weight and Chi2
      const auto firstHitWithCDCKalmanInfoIter = std::find_if(recoHitInformations.begin(),
                                                              recoHitInformations.end(),
                                                              hitHasKalmanInfoAndIsFromCDC);
      std::optional<float> weight_firstCDCHit;
      std::optional<float> smoothedChi2_firstCDCHit;
      if (firstHitWithCDCKalmanInfoIter != recoHitInformations.end()) {
        const genfit::KalmanFitterInfo* kalmanFitterInfo = this->getKalmanFitterInfo(recoTrack, *firstHitWithCDCKalmanInfoIter);
        weight_firstCDCHit = kalmanFitterInfo->getWeights().front();
        smoothedChi2_firstCDCHit = this->getSmoothedChi2(kalmanFitterInfo);
      }
      m_variables.at("weight_firstCDCHit") = weight_firstCDCHit.value_or(m_valueIfNAN);
      m_variables.at("smoothedChi2_firstCDCHit") = smoothedChi2_firstCDCHit.value_or(m_valueIfNAN);

      // find last SVD hit with Kalman info and extract weight and Chi2
      const auto lastHitWithSVDKalmanInfoIter = std::find_if(recoHitInformations.rbegin(),
                                                             recoHitInformations.rend(),
                                                             hitHasKalmanInfoAndIsFromSVD);
      std::optional<float> weight_lastSVDHit;
      std::optional<float> smoothedChi2_lastSVDHit;
      if (lastHitWithSVDKalmanInfoIter != recoHitInformations.rend()) {
        const genfit::KalmanFitterInfo* kalmanFitterInfo = this->getKalmanFitterInfo(recoTrack, *lastHitWithSVDKalmanInfoIter);
        weight_lastSVDHit = kalmanFitterInfo->getWeights().front();
        smoothedChi2_lastSVDHit = this->getSmoothedChi2(kalmanFitterInfo);
      }
      m_variables.at("weight_lastSVDHit") = weight_lastSVDHit.value_or(m_valueIfNAN);
      m_variables.at("smoothedChi2_lastSVDHit") = smoothedChi2_lastSVDHit.value_or(m_valueIfNAN);

      std::vector<float> fitWeights;
      std::vector<float> chi2Values;
      fitWeights.reserve(kalmanFitterInfos.size());
      chi2Values.reserve(kalmanFitterInfos.size());
      for (const auto& kalmanFitterInfo : kalmanFitterInfos) {
        fitWeights.push_back(kalmanFitterInfo->getWeights().front());
        chi2Values.push_back(this->getSmoothedChi2(kalmanFitterInfo).value_or(m_valueIfNAN));
      }
      setStats("weight", fitWeights);
      setStats("smoothedChi2", chi2Values);
    }

  protected:
    /// initialize statistics subsets of variables from clusters that get combined for SPTC
    void initializeStats(const std::string& identifier, std::vector<Named<float*>>& variables)
    {
      addVariable(identifier + "_max", variables);
      addVariable(identifier + "_min", variables);
      addVariable(identifier + "_mean", variables);
      addVariable(identifier + "_std", variables);
      addVariable(identifier + "_median", variables);
      addVariable(identifier + "_n_zeros", variables);
      addVariable(identifier + "_firstCDCHit", variables);
      addVariable(identifier + "_lastSVDHit", variables);
    }

    /// calculated statistics and saves them in variable set
    void setStats
    (const std::string& identifier, std::vector<float>& values)
    {
      int size = values.size();
      if (values.size() == 0) {
        m_variables.at(identifier + "_max") = m_valueIfNAN;
        m_variables.at(identifier + "_min") = m_valueIfNAN;
        m_variables.at(identifier + "_mean") = m_valueIfNAN;
        m_variables.at(identifier + "_std") = m_valueIfNAN;
        m_variables.at(identifier + "_median") = m_valueIfNAN;
        m_variables.at(identifier + "_n_zeros") = m_valueIfNAN;
        m_variables.at(identifier + "_firstCDCHit") = m_valueIfNAN;
        m_variables.at(identifier + "_lastSVDHit") = m_valueIfNAN;
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
      // only in the case size==0 there would be a negative container index, but that case is handled above. So the suppress for cppcheck:
      // cppcheck-suppress negativeContainerIndex
      float median = (size % 2) ? values[size / 2] : 0.5 * (values[size / 2] + values[size / 2 - 1]);
      m_variables.at(identifier + "_median") = median;
    }

  private:
    /// Helper function to get Kalman fitter info from RecoHitInformation if available
    genfit::KalmanFitterInfo* getKalmanFitterInfo(const RecoTrack& recoTrack,
                                                  const RecoHitInformation* recoHitInformation)
    {
      const genfit::TrackPoint* trackPointPtr = recoTrack.getCreatedTrackPoint(recoHitInformation);
      if (trackPointPtr) {
        return trackPointPtr->getKalmanFitterInfo();
      } else {
        return nullptr;
      }
    }
    /// Helper function to safely get Chi2 from a KalmanFitterInfo object if available, and if not
    /// return nullopt
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

    /** Define a default value to use if a variable cannot be calculated. A
     * value different from NAN, but which is not obtainable otherwise, can be
     * useful if one wants the MVA classifier to train on the variable not being
     * available instead of ignoring it. */
    const float m_valueIfNAN = -1.0;
  };
}
