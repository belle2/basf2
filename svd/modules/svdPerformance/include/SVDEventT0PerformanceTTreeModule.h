/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>
#include <svd/dataobjects/SVDCluster.h>

#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /**
   *The module is used to create a TTree to study SVD EventT0.
   */
  class SVDEventT0PerformanceTTreeModule : public Module {

  public:

    /** Constructor */
    SVDEventT0PerformanceTTreeModule();
    /** Register input and output data */
    void initialize() override;
    /** Write the TTree to the file*/
    void terminate() override;
    /** Compute the variables and fill the tree*/
    void event() override;

  private:

    //datastore
    StoreObjPtr<EventT0> m_eventT0; /**< event T0 */
    StoreArray<SVDCluster> m_clusters; /**< svd clusters */

    //rootfile
    std::string m_rootFileName = "";   /**< root file name */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

    std::string m_recoTracksStoreArrayName{"RecoTracks"};  /**< storeArray name of the input and output RecoTracks */

    TTree* m_t = nullptr; /**< tree containing info related to the clusters related to tracks*/

    /* Branches of the tree, one fill per event */
    std::vector<float> m_svdClCharge;      /**< cluster charge */
    std::vector<float> m_svdClSNR;      /**< cluster SNR */
    std::vector<float> m_svdClTime;      /**< cluster time */
    std::vector<float> m_svdClTimeErr;      /**< cluster time error*/
    std::vector<float> m_svdClPos;      /**< cluster position */
    std::vector<float> m_svdClPosErr;      /**< cluster position error */
    std::vector<float> m_svdTrueTime;      /**< true time */
    std::vector<int> m_svdTrkPXDHits; /**< number of PXD hits on the track*/
    std::vector<int> m_svdTrkSVDHits; /**< number of SVD hits on the track*/
    std::vector<int> m_svdTrkCDCHits; /**< number of CDC hits on the track*/
    std::vector<float> m_svdTrkd0;     /**< d0 of the track*/
    std::vector<float> m_svdTrkz0;     /**< z0 of the track*/
    std::vector<float> m_svdTrkp;     /**< p of the track*/
    std::vector<float> m_svdTrkpT;     /**< pT of the track*/
    std::vector<float> m_svdTrkpCM;     /**< pCM of the track*/
    std::vector<int> m_svdLayer;  /**< layer */
    std::vector<int> m_svdLadder; /**< ladder */
    std::vector<int> m_svdSensor; /**< sensor */
    std::vector<int> m_svdSize; /**< size */
    std::vector<int> m_svdisUside; /**< is U-cluster side */
    std::vector<int> m_trkNumber; /**< track number in the event*/

    float m_svdEventT0 = -99; /**< event T0 */
    float m_svdEventT0Err = -99; /**< event T0 */
    int m_nTracks = 0; /**< number of tracks in the event*/
    int m_svdTB = -1; /**< trigger bin */
  };
}

