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

#include <tracking/dataobjects/RecoTrack.h>
#include <svd/dataobjects/SVDCluster.h>

#include <string>

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
    virtual void initialize() override;


    /** event */
    virtual void event() override;


    /** final output  */
    virtual void terminate() override;



  protected:

    void calculateProb(TH2F* signal, TH2F* background, TH2F* probability); /**<compute probability*/
    void calculateError(TH2F* signal, TH2F* background, TH2F* error); /**<compute error*/

    // Data members
    std::string m_svdClustersName; /**< SVDCluster collection name */
    std::string m_recoTracksName; /**< RecoTracks collection name*/

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    StoreArray<RecoTrack> m_recoTracks; /**<RecoTrack store array*/

    int m_binSize = 50;  /**<number of bins in charge distribution*/

    int m_maxClusterSize = 5; /**<max numnber of strips the PDF are separated into*/
    bool m_useLegacyNaming = true; /**<use legacy pdf naming*/

    std::string m_outputFileName = "";/**<output file name*/

    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */
    //for setting up histograms


    std::map<std::string, TH2F* > signalHistMap; /**<map of signal histograms*/
    std::map<std::string, TH2F*> backgroundHistMap; /**< map for background histograms*/

  };
} // end namespace Belle2
