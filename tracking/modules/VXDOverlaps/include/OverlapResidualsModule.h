/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: fdattola                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OVERLAPRESIDUALSMODULE_H
#define OVERLAPRESIDUALSMODULE_H

#pragma once

#include <tracking/dataobjects/RecoHitInformation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TCanvas.h>
#include <pxd/reconstruction/PXDRecoHit.h>



namespace Belle2 {


  /**
   * Overlapping residuals
   */
  class OverlapResidualsModule : public Module {

  public:

    /** Module name */
    OverlapResidualsModule();

    /** Register input and output data */
    virtual void initialize();

    /** Compute the difference of coordinate residuals between two hits in overlapping sensors of a same VXD layer */
    virtual void event();

    /** Ends run*/
    void terminate();

    /** user-defined parameters */
    std::string m_rootFileName;

    /** root file name */
    TFile* m_rootFilePtr = 0;


  private:
    StoreArray<SVDTrueHit> m_truhits;
    StoreArray<PXDTrueHit> m_pxdtruhits;
    StoreArray<RecoTrack> recoTrack;
    StoreArray<PXDCluster> m_pxdcluster;
    StoreArray<SVDCluster> m_svdcluster;


    TH1F* h_U_Res = 0;
    TH1F* h_V_Res = 0;

    TH1F* h_SVDstrips_Mult = 0;

    TH1F* h_U_Cl1Cl2_Res[10] = {0};
    TH1F* h_V_Cl1Cl2_Res[10] = {0};

    TH2F* h_Lyr6[17][6] = {0};
    TH2F* h_Lyr5[13][5] = {0};
    TH2F* h_Lyr4[11][4] = {0};
    TH2F* h_Lyr3[8][3] = {0}; //[Ladder][sensor number]
    TH2F* h_Lyr2[13][3] = {0}; //[Ladder][sensor number]
    TH2F* h_Lyr1[9][3] = {0}; //[Ladder][sensor number]

    TCanvas* c_Lyr6[6] = {0};
    TCanvas* c_Lyr5[5] = {0};
    TCanvas* c_Lyr4[4] = {0};
    TCanvas* c_Lyr3[3] = {0};
    TCanvas* c_Lyr2[3] = {0};
    TCanvas* c_Lyr1[3] = {0};

    TH2F* h_Fit_Lyr6[17][6] = {0};
    TH2F* h_Fit_Lyr5[13][5] = {0};
    TH2F* h_Fit_Lyr4[11][4] = {0};
    TH2F* h_Fit_Lyr3[8][3] = {0}; //[Ladder][sensor number]
    TH2F* h_Fit_Lyr2[13][3] = {0}; //[Ladder][sensor number]
    TH2F* h_Fit_Lyr1[9][3] = {0}; //[Ladder][sensor number]

    TCanvas* c_Fit_Lyr6[6] = {0};
    TCanvas* c_Fit_Lyr5[5] = {0};
    TCanvas* c_Fit_Lyr4[4] = {0};
    TCanvas* c_Fit_Lyr3[3] = {0};
    TCanvas* c_Fit_Lyr2[3] = {0};
    TCanvas* c_Fit_Lyr1[3] = {0};


  };
}

#endif /* OVERLAPRESIDUALS_H */
