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
    float deltaRes_SVD_U = 0, /**< tree t_SVD_U branch deltaRes_SVD_U/F */
          intRes_SVD_U = 0,   /**< tree t_SVD_U branch intRes_SVD_U/F */
          int_SVD_U = 0,      /**< tree t_SVD_U branch int_SVD_U/F */
          intPhi_SVD_U = 0,   /**< tree t_SVD_U branch intPhi_SVD_U/F */
          intZ_SVD_U = 0,     /**< tree t_SVD_U branch intZ_SVD_U */
          extRes_SVD_U = 0,   /**< tree t_SVD_U branch extRes_SVD_U/F */
          ext_SVD_U = 0,      /**< tree t_SVD_U branch ext_SVD_U/F */
          extPhi_SVD_U = 0,   /**< tree t_SVD_U branch extPhi_SVD_U/F */
          extZ_SVD_U = 0;     /**< Branch extZ_SVD_U/F */
    unsigned int intLayer_SVD_U = 0,  /**< tree t_SVD_U branch intLayer_SVD_U/i */
                 intLadder_SVD_U = 0, /**< tree t_SVD_U branch intLadder_SVD_U/i */
                 intSensor_SVD_U = 0, /**< tree t_SVD_U branch intSensor_SVD_U/i */
                 extLayer_SVD_U = 0,  /**< tree t_SVD_U branch extLayer_SVD_U/i */
                 extLadder_SVD_U = 0, /**< tree t_SVD_U branch extLadder_SVD_U/i */
                 extSensor_SVD_U = 0; /**< tree t_SVD_U branch extSensor_SVD_U/i */
    /* Branches of SVD v-clusters tree */
    float deltaRes_SVD_V = 0, /**< tree t_SVD_V branch deltaRes_SVD_V/F */
          intRes_SVD_V = 0,   /**< tree t_SVD_V branch intRes_SVD_V/F */
          int_SVD_V = 0,      /**< tree t_SVD_V branch int_SVD_V/F */
          intPhi_SVD_V = 0,   /**< tree t_SVD_V branch intPhi_SVD_V/F */
          intZ_SVD_V = 0,     /**< tree t_SVD_V branch intZ_SVD_V */
          extRes_SVD_V = 0,   /**< tree t_SVD_V branch extRes_SVD_V/F */
          ext_SVD_V = 0,      /**< tree t_SVD_V branch ext_SVD_V/F */
          extPhi_SVD_V = 0,   /**< tree t_SVD_V branch extPhi_SVD_V/F */
          extZ_SVD_V = 0;     /**< Branch extZ_SVD_V/F */
    unsigned int intLayer_SVD_V = 0,  /**< tree t_SVD_V branch intLayer_SVD_V/i */
                 intLadder_SVD_V = 0, /**< tree t_SVD_V branch intLadder_SVD_V/i */
                 intSensor_SVD_V = 0, /**< tree t_SVD_V branch intSensor_SVD_V/i */
                 extLayer_SVD_V = 0,  /**< tree t_SVD_V branch extLayer_SVD_V/i */
                 extLadder_SVD_V = 0, /**< tree t_SVD_V branch extLadder_SVD_V/i */
                 extSensor_SVD_V = 0; /**< tree t_SVD_V branch extSensor_SVD_V/i */
  };
}
#endif /* OVERLAPRESIDUALS_H */
