/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/dataobjects/SVDCluster.h>

#include <string>

namespace Belle2 {
  /**
   * Imports Clusters of the SVD detector and converts them to spacePoints.
   *
   */
  class SVDTimeGroupingModule : public Module {

  public:



    /** Constructor */
    SVDTimeGroupingModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize() override;


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;


  protected:


    // Data members
    std::string m_svdClustersName = "SVDClusters"; /**< SVDCluster collection name */

    /**
     * the storeArray for svdClusters as member,
     * is faster than recreating link for each event
     *
     */
    StoreArray<SVDCluster> m_svdClusters;

    bool   m_useClusterRawTime  = false; /**< Prepare module to work in raw-time if this
      parameter is set. */

    double m_tRangeLow          = -160.; /**< Expected low range of time histogram [ns]. */
    double m_tRangeHigh         =  160.; /**< Expected high range of time histogram [ns]. */
    int    m_rebinningFactor    =   2; /**< Time bin width is 1/m_rebinningFactor ns. Also disables
            the module if zero. */
    double m_fillSigmaN         = 3.; /**< Number of Gaussian sigmas (= hardcoded resolutions) used
           to fill the time histogram for each cluster. */
    double m_minSigma           = 1.; /**< Lower limit of cluster time sigma for the fit for the
           peak-search [ns]. */
    double m_maxSigma           = 15.; /**< Upper limit of cluster time sigma for the fit for the
            peak-search [ns]. */
    double m_fitRangeHalfWidth  = 5.; /**< Half width of the range in which the fit for the
           peak-search is performed [ns]. */
    double m_removeSigmaN       = 5.; /**< Remove upto this sigma of fitted gaus from histogram. */
    double m_fracThreshold      = 0.05; /**< Minimum fraction of candidates in a peak (wrt to the
             highest peak) considered for fitting in the peak-search. */
    int    m_maxGroups          = 20; /**< maximum number of groups to be accepted. */
    double m_expectedSignalTimeCenter =   0; /**< Expected time of the signal [ns]. */
    double m_expectedSignalTimeMin    = -50; /**< Expected low range of signal hits [ns]. */
    double m_expectedSignalTimeMax    =  50; /**< Expected high range of signal hits [ns]. */
    double m_signalLifetime     = 30.; /**< Group prominence is weighted with exponential weight
            with a lifetime defined by this parameter [ns]. */
    int    m_numberOfSignalGroups = 1; /**< Number of groups expected to contain the signal
            clusters. */
    bool   m_formSigleSignalGroup = false; /**< Assign groupID = 0 to all clusters belonging to
                the signal groups. */
    double m_acceptSigmaN       = 5.;    /**< Clusters are tagged within this of fitted group. */
    bool   m_writeGroupInfo     = true;  /**< Write group info in SVDCluster, otherwise kept empty. */
    bool   m_includeOutOfRangeClusters = true; /**< Assign groups to under and overflow. */

    /*! cls-time resolution based on clsSize -> 0: V, 1: U */
    std::vector<float> m_clsSizeVsSigma[2] = {
      {3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805},
      {6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485}
    };

  };
} // end namespace Belle2
