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
  class SVDTimeGroupComposerModule : public Module {

  public:



    /** Constructor */
    SVDTimeGroupComposerModule();



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
    double m_tRangeHigh         = 160.; /** Half-x range of time histogram */
    double m_AverageCountPerBin = 1.;   /** This sets the bin width of histogram time scale */
    double m_threshold          = 1.;   /** Bin Content bellow this is not considered */
    bool   m_applyCentralLimit  = true; /** Sum adjacent bins many times to form the groups. */
    double m_signalRangeLow     = -40;   /** Expected time range of signal hits. */
    double m_signalRangeHigh    = 40;   /** Expected time range of signal hits. */
    int    m_factor             = 10;    /** Fine divisions of histogram. */
    int    m_iteration          = 10;    /** Number of summations of the histogram. */
    bool   m_gausFill           = true;
    double m_fillSigmaN         = 3.;
    double m_calSigmaN          = 5.;
    double m_accSigmaN          = 3.;
    double m_fracThreshold      = 0.01;
    double m_minSigma           = 1.;
    double m_maxSigma           = 15.;
    double m_timeSpread         = 5.;
    int    m_maxGroups          = 10;
    bool   m_writeGroupInfo     = false;
    std::vector<float> m_clsSizeVsSigma =
    {5.33428, 3.35899, 3.32646, 4.15011, 5.38317, 6.28281, 7.07448, 7.36558, 7.7327, 9.66326};
    bool   m_includeOutOfRangeClusters = true; /** Assign groups to under and overflow. */
    bool   m_useOnlyOneGroup    = false; /** Only one group is kept. */
    bool   m_timeBasedSort      = false; /** Group nearest to zero is kept at the first. */

    // modification parameters
    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
  };
} // end namespace Belle2
