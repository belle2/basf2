/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/trackFindingVXD/utilities/Named.h>
#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <numeric>


namespace Belle2 {
  /// class to extract info from individual clusters and combine for SPTC
  class ClusterInfoExtractor : public VariableExtractor {
  public:
    /** Constructor fills variableSet with variables to be extracted
     * @param variableSet set of variable to be filled
     * @param useTimingInfo whether to use the timing info in clusters
     */
    ClusterInfoExtractor(std::vector<Named<float*>>& variableSet, bool useTimingInfo) :
      VariableExtractor(), m_UseTimingInfo(useTimingInfo)
    {
      initializeStats("charge", variableSet);
      initializeStats("seedCharge", variableSet);
      initializeStats("size", variableSet);
      initializeStats("energyLoss", variableSet);
      if (m_UseTimingInfo) {
        initializeStats("time", variableSet);
        initializeStats("timeSigma", variableSet);
      }

    }

    /// extract variables from SpacePoints
    void extractVariables(std::vector<SpacePoint const*> const& spacePoints)
    {
      std::vector<SVDCluster const*> clusters;
      clusters.reserve(spacePoints.size() * 2);

      for (SpacePoint const* sp : spacePoints) {
        RelationVector<SVDCluster> relatedClusters = sp->getRelationsTo<SVDCluster>("");
        for (const SVDCluster& cluster : relatedClusters) {
          clusters.push_back(&cluster);
        }
      }

      // this is fine as it is pointing to the `relatedClusters`, which are in the datastore..
      // cppcheck-suppress invalidLifetime
      std::vector<float> values(clusters.size());
      for (unsigned int i = 0; i < clusters.size(); ++i) {
        values[i] = clusters[i]->getCharge();
      }
      setStats("charge", values);

      for (unsigned int i = 0; i < clusters.size(); ++i) {
        values[i] = clusters[i]->getSeedCharge();
      }
      setStats("seedCharge", values);


      for (unsigned int i = 0; i < clusters.size(); ++i) {
        values[i] = clusters[i]->getSize();
      }
      setStats("size", values);


      for (unsigned int i = 0; i < clusters.size(); ++i) {
        values[i] = clusters[i]->getCharge() / clusters[i]->getSize();
      }
      setStats("energyLoss", values);

      if (m_UseTimingInfo) {
        for (unsigned int i = 0; i < clusters.size(); ++i) {
          values[i] = clusters[i]->getClsTime();
        }
        setStats("time", values);

        for (unsigned int i = 0; i < clusters.size(); ++i) {
          values[i] = clusters[i]->getClsTimeSigma();
        }
        setStats("timeSigma", values);
      }

    }

  protected:
    /// whether to use timing info from cluster
    bool m_UseTimingInfo;

    /// initialize statistics subsets of variables from clusters that get combined for SPTC
    void initializeStats(const std::string& identifier, std::vector<Named<float*>>& variables)
    {
      addVariable(identifier + "_max", variables);
      addVariable(identifier + "_min", variables);
      addVariable(identifier + "_mean", variables);
      addVariable(identifier + "_std", variables);
      addVariable(identifier + "_sum", variables);
    }

    /// calculated statistics and saves them in variable set
    void setStats(const std::string& identifier, std::vector<float>& values)
    {
      short size = values.size();
      if (values.size() == 0) {
        m_variables.at(identifier + "_max") = NAN;
        m_variables.at(identifier + "_min") = NAN;
        m_variables.at(identifier + "_mean") = NAN;
        m_variables.at(identifier + "_std") = NAN;
        m_variables.at(identifier + "_sum") = NAN;
        return;
      }
      // mean
      float sum = std::accumulate(values.begin(), values.end(), 0.0);
      m_variables.at(identifier + "_sum") = sum;
      float mean = sum / size;
      m_variables.at(identifier + "_mean") = mean;
      // variance and standard deviation
      float variance = std::accumulate(values.begin(), values.end(), 0.0,
      [mean, size](float x, float y) {return x + ((y - mean) * (y - mean)) / (size - 1);});
      float stddev = std::sqrt(variance);
      m_variables.at(identifier + "_std") = stddev;
      //min and max
      float min = *(std::min_element(values.begin(), values.end()));
      float max = *(std::max_element(values.begin(), values.end()));
      m_variables.at(identifier + "_min") = min;
      m_variables.at(identifier + "_max") = max;
    }

  };
}
