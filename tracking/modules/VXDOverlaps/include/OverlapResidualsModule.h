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
    /** Array storing reconstructed tracks */
    StoreArray<RecoTrack> recoTrack;
    /** Array storing PXD clusters */
    StoreArray<PXDCluster> m_pxdcluster;
    /** Array storing SVD clusters */
    StoreArray<SVDCluster> m_svdcluster;
    /** Histogram of VXD ( PXD + SVD ) u-coordinate residual*/
    TH1F* h_U_Res = nullptr;
    /** Histogram of VXD ( PXD + SVD ) v-coordinate residual*/
    TH1F* h_V_Res = nullptr;
    /** Histogram of PXD u-coordinate residual */
    TH1F* h_U_Res_PXD = nullptr;
    /** Histogram of PXD v-coordinate residual */
    TH1F* h_V_Res_PXD = nullptr;
    /** Histogram of SVD u-coordinate residual */
    TH1F* h_U_Res_SVD = nullptr;
    /** Histogram of SVD v-coordinate residual */
    TH1F* h_V_Res_SVD = nullptr;
    /** Histogram of SVD strips multiplicity */
    TH1F* h_SVDstrips_Mult = nullptr;
    /** Histogram of SVD u-coordinate residuals grouped by clusters sizes */
    TH1F* h_U_Cl1Cl2_Res[5] = {nullptr};
    /** Histogram of SVD v-coordinate residuals grouped by clusters sizes */
    TH1F* h_V_Cl1Cl2_Res[5] = {nullptr};
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
    /** Sensor hit-map for layer 6 from fitted u and v coordinates */
    TH2F* h_Fit_Lyr6[17][6] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 5 from fitted u and v coordinates */
    TH2F* h_Fit_Lyr5[13][5] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 4 from fitted u and v coordinates */
    TH2F* h_Fit_Lyr4[11][4] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 3 from fitted u and v coordinates */
    TH2F* h_Fit_Lyr3[8][3] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 2 from fitted u and v coordinates */
    TH2F* h_Fit_Lyr2[13][3] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-map for layer 1 from fitted u and v coordinates */
    TH2F* h_Fit_Lyr1[9][3] = {nullptr}; //[Ladder][sensor number]

  };
}
#endif /* OVERLAPRESIDUALS_H */
