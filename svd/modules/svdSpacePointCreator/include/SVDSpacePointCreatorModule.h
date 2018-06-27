/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <string>

#include <TFile.h>

namespace Belle2 {
  /**
   * Imports Clusters of the SVD detector and converts them to spacePoints.
   *
   */
  class SVDSpacePointCreatorModule : public Module {

  public:



    /** Constructor */
    SVDSpacePointCreatorModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize();


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


    /** final output with mini-feedback */
    virtual void terminate();


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeCounters();


  protected:


    // Data members
    std::string m_svdClustersName; /**< SVDCluster collection name */

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    std::string m_spacePointsName; /**< SpacePoints collection name */

    StoreArray<SpacePoint>
    m_spacePoints; /**< the storeArray for spacePoints as member, is faster than recreating link for each event */

    float m_minClusterTime; /**< clusters with time below this value are not considered to make spacePoints*/

    // modification parameters
    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

    bool m_onlySingleClusterSpacePoints; /**< standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */

    std::string m_inputPDF; /**< File path of root file containing pdf histograms */

    TFile* m_calibrationFile; /** Pointer to root TFile containing PDF histograms */

    //counters for testing
    unsigned int m_TESTERSVDClusterCtr; /**< counts total number of SVDCluster occured */

    unsigned int m_TESTERSpacePointCtr; /**< counts total number of SpacePoints occured */

  };
} // end namespace Belle2
