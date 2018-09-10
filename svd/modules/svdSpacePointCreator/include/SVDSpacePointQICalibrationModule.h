/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: James Webb                                               *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <svd/dataobjects/SVDCluster.h>

#include <string>

#include <TFile.h>
#include <TH2.h>

namespace Belle2 {
  /**
   * Imports Clusters of the SVD detector and converts them to spacePoints.
   *
   */
  class SVDSpacePointQICalibrationModule : public Module {

  public:
    /** Constructor */
    SVDSpacePointQICalibrationModule();



    /** Init the module.
    *
    * prepares histograms and storeArrays.
    */
    virtual void initialize();


    /** event */
    virtual void event();


    /** final output  */
    virtual void terminate();



  protected:

    void calculateProb(TH2F* signal, TH2F* background, TH2F* probability);
    void calculateError(TH2F* signal, TH2F* background, TH2F* error);

    // Data members
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_recoTracksName;

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    StoreArray<RecoTrack>
    m_recoTracks;

    int m_binSize;

    int m_maxClusterSize;
    bool m_useLegacyNaming;

    std::string m_outputFileName;

    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
    //for setting up histograms


    std::map<std::string, TH2F* > signalHistMap;
    std::map<std::string, TH2F*> backgroundHistMap;

  };
} // end namespace Belle2
