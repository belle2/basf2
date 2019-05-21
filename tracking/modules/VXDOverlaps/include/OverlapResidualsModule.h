/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Filippo Dattola                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OVERLAPRESIDUALSMODULE_H
#define OVERLAPRESIDUALSMODULE_H

#include <framework/core/HistoModule.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/Module.h>
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
    /** Histogram of VXD ( PXD + SVD ) u-coordinate residual*/
    TH1F* h_U_DeltaRes = nullptr;
    /** Histogram of VXD ( PXD + SVD ) v-coordinate residual*/
    TH1F* h_V_DeltaRes = nullptr;
    /** Histogram of PXD u-coordinate residual */
    TH1F* h_U_DeltaRes_PXD = nullptr;
    /** Histogram of PXD layer-1 u-coordinate residual */
    TH1F* h_U_DeltaRes_PXD_Lyr1 = nullptr;
    /** Histogram of PXD layer-2 u-coordinate residual */
    TH1F* h_U_DeltaRes_PXD_Lyr2 = nullptr;
    /** Histogram of PXD v-coordinate residual */
    TH1F* h_V_DeltaRes_PXD = nullptr;
    /** Histogram of PXD layer-1 v-coordinate residual */
    TH1F* h_V_DeltaRes_PXD_Lyr1 = nullptr;
    /** Histogram of PXD layer-2 v-coordinate residual */
    TH1F* h_V_DeltaRes_PXD_Lyr2 = nullptr;
    /** Histogram of SVD u-coordinate residual */
    TH1F* h_U_DeltaRes_SVD = nullptr;
    /** Histogram of SVD layer-3 u-coordinate residual */
    TH1F* h_U_DeltaRes_SVD_Lyr3 = nullptr;
    /** Histogram of SVD layer-4 u-coordinate residual */
    TH1F* h_U_DeltaRes_SVD_Lyr4 = nullptr;
    /** Histogram of SVD layer-5 u-coordinate residual */
    TH1F* h_U_DeltaRes_SVD_Lyr5 = nullptr;
    /** Histogram of SVD layer-6 u-coordinate residual */
    TH1F* h_U_DeltaRes_SVD_Lyr6 = nullptr;
    /** Histogram of SVD v-coordinate residual */
    TH1F* h_V_DeltaRes_SVD = nullptr;
    /** Histogram of SVD layer-3 v-coordinate residual */
    TH1F* h_V_DeltaRes_SVD_Lyr3 = nullptr;
    /** Histogram of SVD layer-4 v-coordinate residual */
    TH1F* h_V_DeltaRes_SVD_Lyr4 = nullptr;
    /** Histogram of SVD layer-5 v-coordinate residual */
    TH1F* h_V_DeltaRes_SVD_Lyr5 = nullptr;
    /** Histogram of SVD layer-6 v-coordinate residual */
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
    /** Branches of PXD tree */
    float deltaResU_PXD = 0, intResU_PXD = 0, intResV_PXD = 0, intU_PXD = 0, intV_PXD = 0, intPhi_PXD = 0,
          intZ_PXD = 0, extResU_PXD = 0, extResV_PXD = 0, extU_PXD = 0, extV_PXD = 0, extPhi_PXD = 0, extZ_PXD = 0;
    unsigned int intLayer_PXD = 0, intLadder_PXD = 0, intSensor_PXD = 0, extLayer_PXD = 0, extLadder_PXD = 0, extSensor_PXD = 0;
    /** Branches of SVD u-clusters tree */
    float deltaRes_SVD_U = 0, intRes_SVD_U = 0, int_SVD_U = 0, intPhi_SVD_U = 0, intZ_SVD_U = 0, extRes_SVD_U = 0, ext_SVD_U = 0,
          extPhi_SVD_U = 0, extZ_SVD_U = 0;
    unsigned int intLayer_SVD_U = 0, intLadder_SVD_U = 0, intSensor_SVD_U = 0, extLayer_SVD_U = 0, extLadder_SVD_U = 0,
                 extSensor_SVD_U = 0;
    /** Branches of SVD v-clusters tree */
    float deltaRes_SVD_V = 0, intRes_SVD_V = 0, int_SVD_V = 0, intPhi_SVD_V = 0, intZ_SVD_V = 0, extRes_SVD_V = 0, ext_SVD_V = 0,
          extPhi_SVD_V = 0, extZ_SVD_V = 0;
    unsigned int intLayer_SVD_V = 0, intLadder_SVD_V = 0, intSensor_SVD_V = 0, extLayer_SVD_V = 0, extLadder_SVD_V = 0,
                 extSensor_SVD_V = 0;
  };
}
#endif /* OVERLAPRESIDUALS_H */
