/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /**
   * Class to collect log likelihoods from Clusters from ECL and KLM
   * aimed for output to mdst
   * includes functions to return combined likelihood probability
   */

  class Cluster : public RelationsObject {

  public:

    /**
     * Default constructor: log likelihoods and flags set to 0
     */
    Cluster();

    /**
     * Set log likelihood for a given detector and particle
     * @param det detector enumerator
     * @param cluster cluster
     * @param logl log likelihood
     */
    void setLogLikelihood(Const::EDetector det,
                          const Const::Cluster& cluster,
                          float logl);

    /**
     * Check whether cluster information from a given set of detectors is available
     * @param set a set of cluster detectors
     * @return true if the given set of detectors contributed to the cluster information
     */
    bool isAvailable(Const::ClusterDetectorSet set) const {return  m_detectors.contains(set);}

    /**
     * Return log likelihood for a given detector set and particle
     * @param cluster cluster
     * @param set  a set of cluster detectors to use
     * @return log likelihood
     */
    float getLogL(const Const::Cluster& cluster,
                  Const::ClusterDetectorSet set = Const::ClusterDetectorSet::set()) const;

    /**
     * Return log likelihood difference for a given detector set and clusters
     * @param c1 cluster
     * @param c2 cluster
     * @param set  a set of cluster detectors to use
     * @return log likelihood difference logL_c1 - logL_c2
     */
    float getDeltaLogL(const Const::Cluster& c1,
                       const Const::Cluster& c2,
                       Const::ClusterDetectorSet set = Const::ClusterDetectorSet::set()) const
    {
      return getLogL(c1, set) - getLogL(c2, set);
    }

    /**
     * Return combined likelihood probability for a cluster being c1 and not c2,
     * assuming equal prior probabilities.
     * @param c1 cluster
     * @param c2 cluster
     * @param set  a set of cluster detectors to use
     * @return likelihood probability
     */
    double getProbability(const Const::Cluster& c1,
                          const Const::Cluster& c2,
                          Const::ClusterDetectorSet set = Const::ClusterDetectorSet::set()) const
    {
      return getProbability(c1, c2, 1.0, set);
    }

    /**
     * Return combined likelihood probability for a cluster being c1 and not c2
     * @param c1 cluster
     * @param c2 cluster
     * @param ratio ratio of prior probabilities (c1/c2)
     * @param set  a set of cluster detectors to use
     * @return likelihood probability
     */
    double getProbability(const Const::Cluster& c1,
                          const Const::Cluster& c2,
                          double ratio,
                          Const::ClusterDetectorSet set = Const::ClusterDetectorSet::set()) const;


    /**
     * Return combined likelihood probability for a cluster according to clusterSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param cluster cluster
     * @param fractions array of prior probabilities in the order defined in Const::Cluster
     * @param set  a set of cluster detectors to use
     * @return likelihood probability (a value btw. 0 and 1)
     */
    double getProbability(const Const::Cluster& cluster,
                          const double* fractions = 0,
                          Const::ClusterDetectorSet set = Const::ClusterDetectorSet::set()) const;

    /**
     * Return most likely particle among clusterSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param fractions array of prior probabilities in the order defined in Const::Cluster
     * @param set  a set of PID detectors to use
     * @return particle type
     */
    Const::Cluster getMostLikely(const double* fractions = 0,
                                 Const::ClusterDetectorSet set =
                                   Const::ClusterDetectorSet::set()) const;
    /**
     * Prints the content of a private array of log likelihoods
     */
    void printArray() const;

    /**
     * Return HTML Info of cluster Likelihoods
     */
    std::string getInfoHTML() const override;

  private:

    Const::DetectorSet m_detectors;   /**< set of detectors with cluster information */
    float m_logl[Const::ClusterDetectors::c_size][Const::Cluster::c_SetSize]; /**< log likelihoods */

    /**
     * Calculate likelihood probabilities
     * @param fractions array of prior fractions (not needed to be normalized)
     * @param probabilities array of resulting probabilities
     * @param detSet  a set of cluster detectors to use
     */
    void probability(double probabilities[],
                     const double* fractions,
                     Const::ClusterDetectorSet detSet) const;


    ClassDefOverride(Cluster, 1); /**< Initial version. */

  };

} // end namespace Belle2
