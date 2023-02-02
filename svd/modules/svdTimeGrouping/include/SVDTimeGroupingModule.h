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

#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>

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

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    std::string m_eventLevelTrackingInfoName = ""; /**< Name of the EventLevelTrackingInfo */

    double m_tRangeLow          = -160.; /** Half-x range of time histogram */
    double m_tRangeHigh         =  160.; /** Half-x range of time histogram */
    double m_expSignalLoc       =  0.;   /** Expected location of signal. */
    double m_signalRangeLow     = -50;   /** Expected time range of signal hits. */
    double m_signalRangeHigh    =  50;   /** Expected time range of signal hits. */
    int    m_factor             = 2;     /** Fine divisions of histogram. */
    std::vector<float> m_clsSizeVsSigma[2] = { /** cls-time resolution -> 0: V, 1: U */
      {3.49898, 2.94008, 3.46766, 5.3746, 6.68848, 7.35446, 7.35983, 7.71601, 10.6172, 13.4805},
      {6.53642, 3.76216, 3.30086, 3.95969, 5.49408, 7.07294, 8.35687, 8.94839, 9.23135, 10.485}
    };
    double m_fillSigmaN         = 3.;  /** Fill gaus in histogram bins within this to save time. */
    double m_fracThreshold      = 0.05;  /** Bellow threshold clusters are not searched anymore. */
    double m_minSigma           = 1.;  /** Limit of fit parameter. */
    double m_maxSigma           = 15.;   /** Limit of fit parameter. */
    double m_timeSpread         = 5.;  /** Fit gaussian within this range of the peak. */
    int    m_maxGroups          = 20;  /** Groups are not searched beyond this number. */
    double m_calSigmaN          = 5.;  /** Remove upto this sigma of fitted gaus from histogram. */
    double m_accSigmaN          = 3.;  /** Clusters are tagged within this of fitted group. */
    bool   m_writeGroupInfo     = true; /** Write group info in SVDCluster, otherwise empty. */
    bool   m_signalGroupSelection = false; /** Choose one group near expected signal location. */
    bool   m_flatSignalCut      = false; /** Select all clusters within signal range around first gr. */
    bool   m_includeOutOfRangeClusters = true; /** Assign groups to under and overflow. */
    bool   m_useOnlyOneGroup    = false; /** Only one group is kept. */
    double m_exponentialSort    = 0.; /** Group prominence is weighted with exponential weight. */

    // modification parameters
    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
  };
} // end namespace Belle2
