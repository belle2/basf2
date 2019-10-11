/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <svd/dataobjects/SVDCluster.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

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
    void initialize();
    /** Write the TTrees to the file*/
    void terminate();
    /** Compute the variables and fill the tree*/
    void event();

  private:

    std::string m_rootFileName = "";   /**< root file name */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

    std::string m_recoTracksStoreArrayName{"RecoTracks"};  /**< storeArray name of the input and output RecoTracks */

    TTree* m_t_U = nullptr; /**< tree containing info related to the U side clusters*/
    TTree* m_t_V = nullptr;  /**< tree containing info related to the V side clusters*/
    /* Branches of SVD u-clusters tree */
    float m_svdClCharge = 0;      /**< cluster charge */
    float m_svdClSNR = 0;      /**< cluster SNR */
    float m_svdClTime = 0;      /**< cluster time */
    float m_svdRes = 0;   /**< residual computed by genfit */
    float m_svdClPos = 0;      /**< cluster position */
    float m_svdClPosErr = 0;      /**< cluster position error */
    float m_svdTruePos = -99;      /**< true position */
    float m_svdClPhi = 0;   /**< cluster global phi */
    float m_svdClZ = 0;     /**< cluster global Z */
    float m_svdTrkPos = 0; /**< track position*/
    float m_svdTrkPosErr = 0; /**< track position error*/
    float m_svdTrkQoP = 0; /**< track q/p*/
    float m_svdTrkPrime = 0;     /**< tan of incident angle projected on u,w*/
    unsigned int m_svdLayer = 0;  /**< layer */
    unsigned int m_svdLadder = 0; /**< ladder */
    unsigned int m_svdSensor = 0; /**< sensor */
    unsigned int m_svdSize = 0; /**< size */
  };
}

