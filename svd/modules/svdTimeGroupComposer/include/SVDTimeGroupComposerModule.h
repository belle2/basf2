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

#include <TFile.h>
#include <TH1D.h>

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

    bool   m_useOnlyOneGroup    = false; /** Only one group is kept. */
    bool   m_timeBasedSort      = false; /** Group nearest to zero is kept at the first. */
    double m_xRange             = 160.; /** Half-x range of time histogram */
    double m_AverageCountPerBin = 1.;   /** This sets the bin width of histogram time scale */
    double m_threshold          = 1.;   /** Bin Content bellow this is not considered */

    // modification parameters
    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
  };
} // end namespace Belle2
