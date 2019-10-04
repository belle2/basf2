/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Filippo Dattola                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDPERFORMANCETTREEMODULE_H
#define SVDPERFORMANCETTREEMODULE_H

#include <svd/dataobjects/SVDCluster.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TFile.h>

namespace Belle2 {

  /**The module studies VXD hits from overlapping sensors of a same VXD layer.
   * In particular, it computes residuals differences sensitive to possible detector misalignments.
   *
   * Overlapping residuals
   */
  class SVDPerformanceTTreeModule : public Module {

  public:
    /** Constructor */
    SVDPerformanceTTreeModule();
    /** Register input and output data */
    void initialize();
    /** Write the TTrees to the file*/
    void terminate();
    /** Compute the difference of coordinate residuals between two hits in overlapping sensors of a same VXD layer */
    void event();

    /* user-defined parameters */
    std::string m_rootFileName = "";   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */

  private:
    /** StoreArray name of the input and output RecoTracks */
    std::string m_recoTracksStoreArrayName{"RecoTracks"};
    /** Tree containing global information on SVD u-coordinate overlaps */
    TTree* m_t_U = nullptr;
    /** Tree containing global information on SVD v-coordinate overlaps */
    TTree* m_t_V = nullptr;
    /* Branches of SVD u-clusters tree */
    float m_svdClCharge = 0,      /**< cluster charge */
          m_svdClSNR = 0,      /**< cluster SNR */
          m_svdClTime = 0,      /**< cluster time */
          m_svdRes = 0,   /**< residual computed by genfit */
          m_svdClPos = 0,      /**< cluster position */
          m_svdClPosErr = 0,      /**< cluster position error */
          m_svdTruePos = -99,      /**< true position */
          m_svdClPhi = 0,   /**< cluster global phi */
          m_svdClZ = 0,     /**< cluster global Z */
          m_svdTrkPos = 0, /**< track position*/
          m_svdTrkPosErr = 0, /**< track position error*/
          m_svdTrkQoP = 0, /**< track q/p*/
          m_svdTrkPrime = 0;     /**< tan of incident angle projected on u,w*/
    unsigned int m_svdLayer = 0,  /**< layer */
                 m_svdLadder = 0, /**< ladder */
                 m_svdSensor = 0, /**< sensor */
                 m_svdSize = 0; /**< size */
  };
}
#endif /* SVDPERFORMANCETTREEMODULE_H */
