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
    /** StoreArray name of the input and output RecoTracks */
    std::string m_recoTracksStoreArrayName{"RecoTracks"};
    /** Array storing PXD clusters */
    StoreArray<PXDCluster> m_pxdcluster;
    /** Array storing SVD clusters */
    StoreArray<SVDCluster> m_svdcluster;
    /** Histograms of VXD ( PXD + SVD ) differences of residuals*/
    TH1F* h_U_DeltaRes = nullptr;
    TH1F* h_V_DeltaRes = nullptr;
    /** Histograms of PXD differences of residuals */
    TH1F* h_U_DeltaRes_PXD = nullptr;
    TH1F* h_U_DeltaRes_PXD_Lyr1 = nullptr;
    TH1F* h_U_DeltaRes_PXD_Lyr2 = nullptr;
    TH1F* h_V_DeltaRes_PXD = nullptr;
    TH1F* h_V_DeltaRes_PXD_Lyr1 = nullptr;
    TH1F* h_V_DeltaRes_PXD_Lyr2 = nullptr;
    /** Histograms of SVD differences of residuals */
    TH1F* h_U_DeltaRes_SVD = nullptr;
    TH1F* h_U_DeltaRes_SVD_Lyr3 = nullptr;
    TH1F* h_U_DeltaRes_SVD_Lyr4 = nullptr;
    TH1F* h_U_DeltaRes_SVD_Lyr5 = nullptr;
    TH1F* h_U_DeltaRes_SVD_Lyr6 = nullptr;
    TH1F* h_V_DeltaRes_SVD = nullptr;
    TH1F* h_V_DeltaRes_SVD_Lyr3 = nullptr;
    TH1F* h_V_DeltaRes_SVD_Lyr4 = nullptr;
    TH1F* h_V_DeltaRes_SVD_Lyr5 = nullptr;
    TH1F* h_V_DeltaRes_SVD_Lyr6 = nullptr;
    /** 2D histograms: DeltaRes_u vs phi of VXD overlaps for each layer (1 to 6) */
    TH2F* h_DeltaResUPhi_Lyr6 = nullptr;
    TH2F* h_DeltaResUPhi_Lyr5 = nullptr;
    TH2F* h_DeltaResUPhi_Lyr4 = nullptr;
    TH2F* h_DeltaResUPhi_Lyr3 = nullptr;
    TH2F* h_DeltaResUPhi_Lyr2 = nullptr;
    TH2F* h_DeltaResUPhi_Lyr1 = nullptr;
    /** 2D histograms: DeltaRes_u vs z of VXD overlaps for each layer (1 to 6) */
    TH2F* h_DeltaResUz_Lyr6 = nullptr;
    TH2F* h_DeltaResUz_Lyr5 = nullptr;
    TH2F* h_DeltaResUz_Lyr4 = nullptr;
    TH2F* h_DeltaResUz_Lyr3 = nullptr;
    TH2F* h_DeltaResUz_Lyr2 = nullptr;
    TH2F* h_DeltaResUz_Lyr1 = nullptr;
    /** 2D histograms: DeltaRes_v vs z of VXD overlaps for each layer (1 to 6) */
    TH2F* h_DeltaResVz_Lyr6 = nullptr;
    TH2F* h_DeltaResVz_Lyr5 = nullptr;
    TH2F* h_DeltaResVz_Lyr4 = nullptr;
    TH2F* h_DeltaResVz_Lyr3 = nullptr;
    TH2F* h_DeltaResVz_Lyr2 = nullptr;
    TH2F* h_DeltaResVz_Lyr1 = nullptr;
    /** 2D histograms: DeltaRes_v vs phi of VXD overlaps for each layer (1 to 6) */
    TH2F* h_DeltaResVPhi_Lyr6 = nullptr;
    TH2F* h_DeltaResVPhi_Lyr5 = nullptr;
    TH2F* h_DeltaResVPhi_Lyr4 = nullptr;
    TH2F* h_DeltaResVPhi_Lyr3 = nullptr;
    TH2F* h_DeltaResVPhi_Lyr2 = nullptr;
    TH2F* h_DeltaResVPhi_Lyr1 = nullptr;
    /** Histograms of SVD strips multiplicity */
    TH1F* h_SVDstrips_Mult = nullptr;
    /** Histograms of SVD differences of residuals grouped by clusters sizes */
    TH1F* h_U_Cl1Cl2_DeltaRes[5] = {nullptr};
    TH1F* h_V_Cl1Cl2_DeltaRes[5] = {nullptr};
    /** Sensor hit-maps from reconstructed u and v coordinates */
    TH2F* h_Lyr6[17][6] = {nullptr};
    TH2F* h_Lyr5[13][5] = {nullptr};
    TH2F* h_Lyr4[11][4] = {nullptr};
    TH2F* h_Lyr3[8][3] = {nullptr}; //[Ladder][sensor number]
    TH2F* h_Lyr2[13][3] = {nullptr}; //[Ladder][sensor number]
    TH2F* h_Lyr1[9][3] = {nullptr}; //[Ladder][sensor number]
    /** Sensor hit-maps from fitted u and v coordinates */
    TH2F* h_Fit_Lyr6[17][6] = {nullptr};
    TH2F* h_Fit_Lyr5[13][5] = {nullptr};
    TH2F* h_Fit_Lyr4[11][4] = {nullptr};
    TH2F* h_Fit_Lyr3[8][3] = {nullptr}; //[Ladder][sensor number]
    TH2F* h_Fit_Lyr2[13][3] = {nullptr}; //[Ladder][sensor number]
    TH2F* h_Fit_Lyr1[9][3] = {nullptr}; //[Ladder][sensor number]

  };
}
#endif /* OVERLAPRESIDUALS_H */
