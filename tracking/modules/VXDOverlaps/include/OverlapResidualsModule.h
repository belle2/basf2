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


#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <pxd/reconstruction/PXDRecoHit.h>



namespace Belle2 {


  /**The module studies VXD hits from overlapping sensors of a same VXD layer.
   * In particular, it computes residuals differences sensitive to possible detector misalignments.
   *
   * Overlapping residuals
   */
  class OverlapResidualsModule : public Module {

  public:

    /** Constructor */
    OverlapResidualsModule();

    /** Register input and output data */
    virtual void initialize() override;

    /** Compute the difference of coordinate residuals between two hits in overlapping sensors of a same VXD layer */
    virtual void event() override;

    /** Terminate basf2*/
    void terminate() override;

    /** Root filename */
    std::string m_rootFileName;

    /** Pointer to the root file */
    TFile* m_rootFilePtr = NULL;


  private:
    StoreArray<SVDTrueHit> m_truhits;
    StoreArray<PXDTrueHit> m_pxdtruhits;
    StoreArray<RecoTrack> recoTrack;
    StoreArray<PXDCluster> m_pxdcluster;
    StoreArray<SVDCluster> m_svdcluster;


    TH1F* h_U_Res = NULL;
    TH1F* h_V_Res = NULL;

    TH1F* h_SVDstrips_Mult = NULL;

    TH1F* h_U_Cl1Cl2_Res[10] = {NULL};
    TH1F* h_V_Cl1Cl2_Res[10] = {NULL};

    TH2F* h_Lyr6[17][6] = {NULL};
    TH2F* h_Lyr5[13][5] = {NULL};
    TH2F* h_Lyr4[11][4] = {NULL};
    TH2F* h_Lyr3[8][3] = {NULL}; //[Ladder][sensor number]
    TH2F* h_Lyr2[13][3] = {NULL}; //[Ladder][sensor number]
    TH2F* h_Lyr1[9][3] = {NULL}; //[Ladder][sensor number]

    TH2F* h_Fit_Lyr6[17][6] = {NULL};
    TH2F* h_Fit_Lyr5[13][5] = {NULL};
    TH2F* h_Fit_Lyr4[11][4] = {NULL};
    TH2F* h_Fit_Lyr3[8][3] = {NULL}; //[Ladder][sensor number]
    TH2F* h_Fit_Lyr2[13][3] = {NULL}; //[Ladder][sensor number]
    TH2F* h_Fit_Lyr1[9][3] = {NULL}; //[Ladder][sensor number]

  };
}

#endif /* OVERLAPRESIDUALS_H */
