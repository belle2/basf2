/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dataobjects/EventT0.h>
#include <framework/dbobjects/HardwareClockSettings.h>

#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /**The module is used to create a TTree to study SVD clusters, genfit unbiased residuals and many other properties related to the track they belong to.
   *
   */
  class SVDPerformanceTTreeModule : public Module {

  public:
    /** Constructor */
    SVDPerformanceTTreeModule();
    /** Register input and output data */
    void initialize() override;
    /** Write the TTrees to the file*/
    void terminate() override;
    /** Compute the variables and fill the tree*/
    void event() override;
    /** Compute the APV clock period*/
    void beginRun() override;

  private:

    StoreObjPtr<EventT0> m_EventT0; /**< event T0 */

    /** Hardware Clocks*/
    DBObjPtr<HardwareClockSettings> m_hwClock;
    /** APV clock period*/
    double m_apvClockPeriod = std::numeric_limits<double>::quiet_NaN();

    std::string m_rootFileName = "";   /**< root file name */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

    std::string m_recoTracksStoreArrayName{"RecoTracks"};  /**< storeArray name of the input and output RecoTracks */

    TTree* m_t_U = nullptr; /**< tree containing info related to the U side clusters*/
    TTree* m_t_V = nullptr;  /**< tree containing info related to the V side clusters*/
    /* event-wise branches*/
    float m_cdcEventT0 = std::numeric_limits<float>::quiet_NaN(); /**< CDC event T0 */
    float m_cdcEventT0_6SRF = std::numeric_limits<float>::quiet_NaN(); /**< CDC event T0 in the 6-sample SVD ref frame*/
    float m_cdcEventT0_3SRF = std::numeric_limits<float>::quiet_NaN(); /**< CDC event T0 in the 3-sample SVD ref frame*/
    float m_cdcEventT0Err = std::numeric_limits<float>::quiet_NaN(); /**< CDC event T0 Error */
    unsigned int m_svdTB = 0; /**< trigger bin */
    /* Branches of SVD u and v clusters tree */
    float m_svdClCharge = 0;      /**< cluster charge */
    float m_svdClSNR = 0;      /**< cluster SNR */
    float m_svdClTime = 0;      /**< cluster time */
    float m_svdClTime_6SRF = 0;      /**< cluster time in the 6-sample SVD ref frame*/
    float m_svdClTime_3SRF = 0;      /**< cluster time in the 3-sample SVD ref frame*/
    float m_svdRes = 0;   /**< residual computed by genfit */
    float m_svdPitch = 0; /**< svd pitch */
    float m_svdWidth = 0; /**< svd sensor width */
    float m_svdLength = 0; /**< svd sensor length */
    float m_svdClIntStrPos = 0;      /**< cluster interstrip position */
    float m_svdClPos = 0;      /**< cluster position */
    float m_svdClPosErr = 0;      /**< cluster position error */
    float m_svdTruePos = -99;      /**< true position */
    float m_svdClPhi = 0;   /**< cluster global phi */
    float m_svdClZ = 0;     /**< cluster global Z */
    std::vector<float> m_svdStripCharge; /**< charge of the strips of the cluster*/
    std::vector<float> m_svdStrip6Samples; /**< 6 samples of the strips of the cluster*/
    std::vector<float> m_svdStripTime; /**< time of the strips of the cluster*/
    std::vector<float> m_svdStripPosition; /**< absolute position of the strips of the cluster*/
    int m_svdTrkPXDHits = 0; /**< number of PXD hits on the track*/
    int m_svdTrkSVDHits = 0; /**< number of SVD hits on the track*/
    int m_svdTrkCDCHits = 0; /**< number of CDC hits on the track*/
    float m_svdTrkd0 = 0;     /**< d0 of the track*/
    float m_svdTrkz0 = 0;     /**< z0 of the track*/
    float m_svdTrkpT = 0;     /**< pT of the track*/
    float m_svdTrkpCM = 0;     /**< pCM of the track*/
    float m_svdTrkTraversedLength = 0; /**< traversed length of the track in the sensor*/
    float m_svdTrkPos = 0; /**< track position*/
    float m_svdTrkPosOS = 0; /**< track position on the other side*/
    float m_svdTrkPosErr = 0; /**< track position error*/
    float m_svdTrkPosErrOS = 0; /**< track position error on the other side*/
    float m_svdTrkQoP = 0; /**< track q/p*/
    float m_svdTrkPrime = 0;     /**< tan of incident angle projected on u/v,w*/
    float m_svdTrkPrimeOS = 0;     /**< tan of incident angle projected on v/u,w (other side)*/
    float m_svdTrkPosUnbiased = 0; /**< unbiased track position */
    float m_svdTrkPosErrUnbiased = 0; /**< unbiased track position error*/
    float m_svdTrkQoPUnbiased = 0; /**< unbiased track q/p*/
    float m_svdTrkPrimeUnbiased = 0;     /**< unbiased tan of incident angle projected on u,w*/
    unsigned int m_svdLayer = 0;  /**< layer */
    unsigned int m_svdLadder = 0; /**< ladder */
    unsigned int m_svdSensor = 0; /**< sensor */
    unsigned int m_svdSize = 0; /**< size */
    unsigned int m_svdFF = 0; /**< first frame */

  };
}

