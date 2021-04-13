/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Filippo Dattola                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

namespace Belle2 {
  /**The module studies VXD hits from overlapping sensors of a same VXD layer.
   * In particular, it computes residuals differences sensitive to possible detector misalignments.
   *
   * Overlapping residuals
   */
  class OverlapResidualsModule : public HistoModule {  // <- derived from HistoModule

  public:
    /** Constructor */
    OverlapResidualsModule();
    /** Register input and output data */
    void initialize() override;
    /** Compute the difference of coordinate residuals between two hits in overlapping sensors of a same VXD layer */
    void event() override;
    /**  Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed to be placed in this function */
    void defineHisto() override;

  private:
    /** Expert level switch */
    bool m_ExpertLevel;
    /** StoreArray name of the input and output RecoTracks */
    std::string m_recoTracksStoreArrayName{"RecoTracks"};
    /** Array storing PXD clusters */
    StoreArray<PXDCluster> m_pxdcluster;
    /** Array storing SVD clusters */
    StoreArray<SVDCluster> m_svdcluster;
    /** Histogram of VXD ( PXD + SVD ) differences of u-coordinate residuals*/
    TH1F* h_U_DeltaRes = nullptr;
    /** Histogram of VXD ( PXD + SVD ) differences of v-coordinate residuals*/
    TH1F* h_V_DeltaRes = nullptr;
    /** Histogram of PXD u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_PXD = nullptr;
    /** Histogram of PXD layer-1 u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_PXD_Lyr1 = nullptr;
    /** Histogram of PXD layer-2 u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_PXD_Lyr2 = nullptr;
    /** Histogram of PXD v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_PXD = nullptr;
    /** Histogram of PXD layer-1 v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_PXD_Lyr1 = nullptr;
    /** Histogram of PXD layer-2 v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_PXD_Lyr2 = nullptr;
    /** Histogram of SVD u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_SVD = nullptr;
    /** Histogram of SVD layer-3 u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_SVD_Lyr3 = nullptr;
    /** Histogram of SVD layer-4 u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_SVD_Lyr4 = nullptr;
    /** Histogram of SVD layer-5 u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_SVD_Lyr5 = nullptr;
    /** Histogram of SVD layer-6 u-coordinate differences of residuals */
    TH1F* h_U_DeltaRes_SVD_Lyr6 = nullptr;
    /** Histogram of SVD v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_SVD = nullptr;
    /** Histogram of SVD layer-3 v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_SVD_Lyr3 = nullptr;
    /** Histogram of SVD layer-4 v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_SVD_Lyr4 = nullptr;
    /** Histogram of SVD layer-5 v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_SVD_Lyr5 = nullptr;
    /** Histogram of SVD layer-6 v-coordinate differences of residuals */
    TH1F* h_V_DeltaRes_SVD_Lyr6 = nullptr;
    /** 2D histogram of DeltaRes_u vs phi of VXD overlaps for layer 6 */
    TH2F* h_DeltaResUPhi_Lyr6 = nullptr;
    /** 2D histogram of DeltaRes_u vs phi of VXD overlaps for layer 5 */
    TH2F* h_DeltaResUPhi_Lyr5 = nullptr;
    /** 2D histogram of DeltaRes_u vs phi of VXD overlaps for layer 4 */
    TH2F* h_DeltaResUPhi_Lyr4 = nullptr;
    /** 2D histogram of DeltaRes_u vs phi of VXD overlaps for layer 3 */
    TH2F* h_DeltaResUPhi_Lyr3 = nullptr;
    /** 2D histogram of DeltaRes_u vs phi of VXD overlaps for layer 2 */
    TH2F* h_DeltaResUPhi_Lyr2 = nullptr;
    /** 2D Histogram of DeltaRes_u vs phi of VXD overlaps for layer 1 */
    TH2F* h_DeltaResUPhi_Lyr1 = nullptr;
    /** 2D histogram of DeltaRes_u vs z of VXD overlaps for layer 6 */
    TH2F* h_DeltaResUz_Lyr6 = nullptr;
    /** 2D histogram of DeltaRes_u vs z of VXD overlaps for layer 5 */
    TH2F* h_DeltaResUz_Lyr5 = nullptr;
    /** 2D histogram of DeltaRes_u vs z of VXD overlaps for layer 4 */
    TH2F* h_DeltaResUz_Lyr4 = nullptr;
    /** 2D histogram of DeltaRes_u vs z of VXD overlaps for layer 3 */
    TH2F* h_DeltaResUz_Lyr3 = nullptr;
    /** 2D histogram of DeltaRes_u vs z of VXD overlaps for layer 2 */
    TH2F* h_DeltaResUz_Lyr2 = nullptr;
    /** 2D histogram of DeltaRes_u vs z of VXD overlaps for layer 1 */
    TH2F* h_DeltaResUz_Lyr1 = nullptr;
    /** 2D histogram of DeltaRes_v vs z of VXD overlaps for layer 6 */
    TH2F* h_DeltaResVz_Lyr6 = nullptr;
    /** 2D histogram of DeltaRes_v vs z of VXD overlaps for layer 5 */
    TH2F* h_DeltaResVz_Lyr5 = nullptr;
    /** 2D histogram of DeltaRes_v vs z of VXD overlaps for layer 4 */
    TH2F* h_DeltaResVz_Lyr4 = nullptr;
    /** 2D histogram of DeltaRes_v vs z of VXD overlaps for layer 3 */
    TH2F* h_DeltaResVz_Lyr3 = nullptr;
    /** 2D histogram of DeltaRes_v vs z of VXD overlaps for layer 2 */
    TH2F* h_DeltaResVz_Lyr2 = nullptr;
    /** 2D histogram of DeltaRes_v vs z of VXD overlaps for layer 1 */
    TH2F* h_DeltaResVz_Lyr1 = nullptr;
    /** 2D histogram of DeltaRes_v vs phi of VXD overlaps for layer 6 */
    TH2F* h_DeltaResVPhi_Lyr6 = nullptr;
    /** 2D histogram of DeltaRes_v vs phi of VXD overlaps for layer 5 */
    TH2F* h_DeltaResVPhi_Lyr5 = nullptr;
    /** 2D histogram of DeltaRes_v vs phi of VXD overlaps for layer 4 */
    TH2F* h_DeltaResVPhi_Lyr4 = nullptr;
    /** 2D histogram of DeltaRes_v vs phi of VXD overlaps for layer 3 */
    TH2F* h_DeltaResVPhi_Lyr3 = nullptr;
    /** 2D histogram of DeltaRes_v vs phi of VXD overlaps for layer 2 */
    TH2F* h_DeltaResVPhi_Lyr2 = nullptr;
    /** 2D histogram of DeltaRes_v vs phi of VXD overlaps for layer 1 */
    TH2F* h_DeltaResVPhi_Lyr1 = nullptr;
    /** Histogram of SVD strips multiplicity */
    TH1F* h_SVDstrips_Mult = nullptr;
    /** Histogram of SVD differences of u-coordinate residuals grouped by clusters sizes */
    TH1F* h_U_Cl1Cl2_DeltaRes[5] = {nullptr};
    /** Histogram of SVD differences of v-coordinate residuals grouped by clusters sizes */
    TH1F* h_V_Cl1Cl2_DeltaRes[5] = {nullptr};
    /** Sensor hit-map for layer 6 from reconstructed u and v coordinates */
    TH2F* h_Lyr6[17][6] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 5 from reconstructed u and v coordinates */
    TH2F* h_Lyr5[13][5] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 4 from reconstructed u and v coordinates */
    TH2F* h_Lyr4[11][4] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 3 from reconstructed u and v coordinates */
    TH2F* h_Lyr3[8][3] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 2 from reconstructed u and v coordinates */
    TH2F* h_Lyr2[13][3] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 1 from reconstructed u and v coordinates */
    TH2F* h_Lyr1[9][3] = {nullptr}; //[Ladder][sensor number]
    /** Tree containing global information on PXD overlaps */
    TTree* t_PXD = nullptr;
    /** Tree containing global information on SVD u-coordinate overlaps */
    TTree* t_SVD_U = nullptr;
    /** Tree containing global information on SVD v-coordinate overlaps */
    TTree* t_SVD_V = nullptr;
    /* Branches of PXD tree */
    float deltaResU_PXD = 0, /**< tree t_PXD branch deltaResU/F */
          intResU_PXD = 0,   /**< tree t_PXD branch intResU_PXD/F */
          intResV_PXD = 0,   /**< tree t_PXD branch intResV/PXD/F */
          intU_PXD = 0,      /**< tree t_PXD branch intU_PXD/F */
          intV_PXD = 0,      /**< tree t_PXD branch intV_PXD/F */
          intPhi_PXD = 0,    /**< tree t_PXD branch intPhi_PXD/F */
          intZ_PXD = 0,      /**< tree t_PXD branch intZ_PXD/F */
          extResU_PXD = 0,   /**< tree t_PXD branch extResU_PXD/F */
          extResV_PXD = 0,   /**< tree t_PXD branch extResV_PXD/F */
          extU_PXD = 0,      /**< tree t_PXD branch extU_PXD/F */
          extV_PXD = 0,      /**< tree t_PXD branch extV_PXD/F */
          extPhi_PXD = 0,    /**< tree t_PXD branch extPhi/PXD/F */
          extZ_PXD = 0;      /**< tree t_PXD branch extZ_PXD/F */
    unsigned int intLayer_PXD = 0,  /**< tree t_PXD branch intLayer_PXD/i */
                 intLadder_PXD = 0, /**< tree t_PXD branch intLadder_PXD/i */
                 intSensor_PXD = 0, /**< tree t_PXD branch intSensor_PXD/i */
                 extLayer_PXD = 0,  /**< tree t_PXD branch extLayer_PXD/i */
                 extLadder_PXD = 0, /**< tree t_PXD branch extLadder_PXD/i */
                 extSensor_PXD = 0; /**< tree t_PXD branch extSensor_PXD/i */
    /* Branches of SVD u-clusters tree */
    float svdDeltaRes_U = 0,          /**< U difference between external and internal residual */
          svdClCharge_U_int = 0,      /**< U internal cluster charge */
          svdClSNR_U_int = 0,         /**< U internal cluster SNR */
          svdClTime_U_int = 0,        /**< U internal cluster time */
          svdRes_U_int = 0,           /**< U internal residual computed by genfit */
          svdClIntStrPos_U_int = 0,   /**< U internal cluster interstrip position */
          svdClPos_U_int = 0,         /**< U internal cluster position */
          svdClPosErr_U_int = 0,      /**< U internal cluster position error */
          svdTruePos_U_int = -99,     /**< U internal true position */
          svdClPhi_U_int = 0,         /**< U internal cluster global phi */
          svdClZ_U_int = 0,           /**< U internal cluster global Z */
          svdTrkd0 = 0,         /**< d0 of the track */
          svdTrkz0 = 0,         /**< z0 of the track */
          svdTrkpT = 0,         /**< pT of the track */
          svdTrkpCM = 0,        /**< pCM of the track */
          svdTrkTraversedLength_U_int = 0,   /**< U internal traversed length of the track in the sensor */
          svdTrkPos_U_int = 0,        /**< U internal track position */
          svdTrkPosOS_U_int = 0,      /**< U internal track position on the other side */
          svdTrkPosErr_U_int = 0,     /**< U internal track position error */
          svdTrkPosErrOS_U_int = 0,   /**< U internal track position error on the other side */
          svdTrkQoP_U_int = 0,        /**< U internal track q/p */
          svdTrkPrime_U_int = 0,      /**< U internal tan of incident angle projected on u,w */
          svdTrkPrimeOS_U_int = 0,    /**< U internal tan of incident angle projected on v/u,w (other side) */
          svdTrkPosUnbiased_U_int = 0,   /**< U internal unbiased track position */
          svdTrkPosErrUnbiased_U_int = 0,   /**< U internal unbiased track position error */
          svdTrkQoPUnbiased_U_int = 0,   /**< U internal unbiased track q/p */
          svdTrkPrimeUnbiased_U_int = 0,   /**< U internal unbiased tan of incident angle projected on u,w */
          svdClCharge_U_ext = 0,      /**< U external cluster charge */
          svdClSNR_U_ext = 0,         /**< U external cluster SNR */
          svdClTime_U_ext = 0,        /**< U external cluster time */
          svdRes_U_ext = 0,           /**< U external residual computed by genfit */
          svdClIntStrPos_U_ext = 0,   /**< U external cluster interstrip position */
          svdClPos_U_ext = 0,         /**< U external cluster position */
          svdClPosErr_U_ext = 0,      /**< U external cluster position error */
          svdTruePos_U_ext = -99,     /**< U external true position */
          svdClPhi_U_ext = 0,         /**< U external cluster global phi */
          svdClZ_U_ext = 0,           /**< U external cluster global Z */
          svdTrkTraversedLength_U_ext = 0,   /**< U external traversed length of the track in the sensor */
          svdTrkPos_U_ext = 0,        /**< U external track position */
          svdTrkPosOS_U_ext = 0,      /**< U external track position on the other side */
          svdTrkPosErr_U_ext = 0,     /**< U external track position error */
          svdTrkPosErrOS_U_ext = 0,   /**< U external track position error on the other side */
          svdTrkQoP_U_ext = 0,        /**< U external track q/p */
          svdTrkPrime_U_ext = 0,      /**< U external tan of incident angle projected on u,w */
          svdTrkPrimeOS_U_ext = 0,    /**< U external tan of incident angle projected on v/u,w (other side) */
          svdTrkPosUnbiased_U_ext = 0,   /**< U external unbiased track position */
          svdTrkPosErrUnbiased_U_ext = 0,   /**< U external unbiased track position error */
          svdTrkQoPUnbiased_U_ext = 0,   /**< U external unbiased track q/p */
          svdTrkPrimeUnbiased_U_ext = 0;   /**< U external unbiased tan of incident angle projected on u,w */
    int svdTrkPXDHits = 0,   /**< number of PXD hits on the track*/
        svdTrkSVDHits = 0,   /**< number of PXD hits on the track*/
        svdTrkCDCHits = 0;   /**< number of PXD hits on the track*/
    unsigned int svdLayer_U_int = 0,    /**< U internal layer */
                 svdLadder_U_int = 0,   /**< U internal ladder */
                 svdSensor_U_int = 0,   /**< U internal sensor */
                 svdSize_U_int = 0,     /**< U internal size */
                 svdLayer_U_ext = 0,    /**< U external layer */
                 svdLadder_U_ext = 0,   /**< U external ladder */
                 svdSensor_U_ext = 0,   /**< U external sensor */
                 svdSize_U_ext = 0;     /**< U external size */
    std::vector<float> svdStripCharge_U_int,     /**< U internal charge of the strips of the cluster*/
        svdStrip6Samples_U_int,   /**< 6 samples of the strips of the cluster*/
        svdStripTime_U_int,       /**< U internal time of the strips of the cluster*/
        svdStripPosition_U_int,   /**< U internal position of the strips of the cluster*/
        svdStripCharge_U_ext,     /**< U external charge of the strips of the cluster*/
        svdStrip6Samples_U_ext,   /**< 6 samples of the strips of the cluster*/
        svdStripTime_U_ext,       /**< U external time of the strips of the cluster*/
        svdStripPosition_U_ext;   /**< U external position of the strips of the cluster*/
    /* Branches of SVD v-clusters tree */
    float svdDeltaRes_V = 0,          /**< V difference between external and internal residual */
          svdClCharge_V_int = 0,      /**< V internal cluster charge */
          svdClSNR_V_int = 0,         /**< V internal cluster SNR */
          svdClTime_V_int = 0,        /**< V internal cluster time */
          svdRes_V_int = 0,           /**< V internal residual computed by genfit */
          svdClIntStrPos_V_int = 0,   /**< V internal cluster interstrip position */
          svdClPos_V_int = 0,         /**< V internal cluster position */
          svdClPosErr_V_int = 0,      /**< V internal cluster position error */
          svdTruePos_V_int = -99,     /**< V internal true position */
          svdClPhi_V_int = 0,         /**< V internal cluster global phi */
          svdClZ_V_int = 0,           /**< V internal cluster global Z */
          svdTrkTraversedLength_V_int = 0,   /**< V internal traversed length of the track in the sensor */
          svdTrkPos_V_int = 0,        /**< V internal track position */
          svdTrkPosOS_V_int = 0,      /**< V internal track position on the other side */
          svdTrkPosErr_V_int = 0,     /**< V internal track position error */
          svdTrkPosErrOS_V_int = 0,   /**< V internal track position error on the other side */
          svdTrkQoP_V_int = 0,        /**< V internal track q/p */
          svdTrkPrime_V_int = 0,      /**< V internal tan of incident angle projected on u,w */
          svdTrkPrimeOS_V_int = 0,    /**< V internal tan of incident angle projected on v/u,w (other side) */
          svdTrkPosUnbiased_V_int = 0,   /**< V internal unbiased track position */
          svdTrkPosErrUnbiased_V_int = 0,   /**< V internal unbiased track position error */
          svdTrkQoPUnbiased_V_int = 0,   /**< V internal unbiased track q/p */
          svdTrkPrimeUnbiased_V_int = 0,   /**< V internal unbiased tan of incident angle projected on u,w */
          svdClCharge_V_ext = 0,      /**< V external cluster charge */
          svdClSNR_V_ext = 0,         /**< V external cluster SNR */
          svdClTime_V_ext = 0,        /**< V external cluster time */
          svdRes_V_ext = 0,           /**< V external residual computed by genfit */
          svdClIntStrPos_V_ext = 0,   /**< V external cluster interstrip position */
          svdClPos_V_ext = 0,         /**< V external cluster position */
          svdClPosErr_V_ext = 0,      /**< V external cluster position error */
          svdTruePos_V_ext = -99,     /**< V external true position */
          svdClPhi_V_ext = 0,         /**< V external cluster global phi */
          svdClZ_V_ext = 0,           /**< V external cluster global Z */
          svdTrkTraversedLength_V_ext = 0,   /**< V external traversed length of the track in the sensor */
          svdTrkPos_V_ext = 0,        /**< V external track position */
          svdTrkPosOS_V_ext = 0,      /**< V external track position on the other side */
          svdTrkPosErr_V_ext = 0,     /**< V external track position error */
          svdTrkPosErrOS_V_ext = 0,   /**< V external track position error on the other side */
          svdTrkQoP_V_ext = 0,        /**< V external track q/p */
          svdTrkPrime_V_ext = 0,      /**< V external tan of incident angle projected on u,w */
          svdTrkPrimeOS_V_ext = 0,    /**< V external tan of incident angle projected on v/u,w (other side) */
          svdTrkPosUnbiased_V_ext = 0,   /**< V external unbiased track position */
          svdTrkPosErrUnbiased_V_ext = 0,   /**< V external unbiased track position error */
          svdTrkQoPUnbiased_V_ext = 0,   /**< V external unbiased track q/p */
          svdTrkPrimeUnbiased_V_ext = 0;   /**< V external unbiased tan of incident angle projected on u,w */
    unsigned int svdLayer_V_int = 0,    /**< V internal layer */
                 svdLadder_V_int = 0,   /**< V internal ladder */
                 svdSensor_V_int = 0,   /**< V internal sensor */
                 svdSize_V_int = 0,     /**< V internal size */
                 svdLayer_V_ext = 0,    /**< V external layer */
                 svdLadder_V_ext = 0,   /**< V external ladder */
                 svdSensor_V_ext = 0,   /**< V external sensor */
                 svdSize_V_ext = 0;     /**< V external size */
    std::vector<float> svdStripCharge_V_int,     /**< V internal charge of the strips of the cluster*/
        svdStrip6Samples_V_int,   /**< 6 samples of the strips of the cluster*/
        svdStripTime_V_int,       /**< V internal time of the strips of the cluster*/
        svdStripPosition_V_int,   /**< V internal position of the strips of the cluster*/
        svdStripCharge_V_ext,     /**< V external charge of the strips of the cluster*/
        svdStrip6Samples_V_ext,   /**< 6 samples of the strips of the cluster*/
        svdStripTime_V_ext,       /**< V external time of the strips of the cluster*/
        svdStripPosition_V_ext;   /**< V external position of the strips of the cluster*/
  };
}
