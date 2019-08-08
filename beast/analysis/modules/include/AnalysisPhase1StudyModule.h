/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ANALYSISPHASE1STUDYMODULE_H
#define ANALYSISPHASE1STUDYMODULE_H

#include <framework/core/HistoModule.h>
#include <string>
#include <vector>


#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>
#include "TTree.h"

namespace Belle2 {
  //  namespace analysis {

  /**
   * Study module for BEAST
   *
   * Produces histograms from BEAST simulation.   *
   */
  class AnalysisPhase1StudyModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    AnalysisPhase1StudyModule();

    /**  */
    virtual ~AnalysisPhase1StudyModule();

    /**  */
    virtual void initialize() override;

    /**  */
    virtual void beginRun() override;

    /**  */
    virtual void event() override;

    /**  */
    virtual void endRun() override;

    /**  */
    virtual void terminate() override;

    /** Defines the histograms*/
    virtual void defineHisto() override;

  private:

    /** counter */
    TH1F* h_count;
    /** r v z */
    TH2F* h_prodvtx[10];
    /** r v z */
    TH2F* h_decavtx[10];
    /** kin v z */
    TH2F* h_kineticvz[10];
    TH2F* h_kineticvz1[10];
    TH2F* h_kineticvz2[10];
    /** kin v z */
    TH2F* h_kineticvz_zoom[10];
    /** kin v z */
    TH2F* h_Wkineticvz[10];
    /** kin v z */
    TH2F* h_Wkineticvz_zoom[10];
    /** theta v z */
    TH2F* h_thetavz[10];
    /** phi v z */
    TH2F* h_phivz[10];
    TH2F* h_phive[10];
    TH2F* h_rve[10];
    /** sad xy */
    TH2F* h_sad_xy[2];
    /** g4 xy */
    TH2F* h_g4_xy;
    /** sad rate */
    TH1F* h_sad_sir[2];
    /** sad rate */
    TH1F* h_sad_sall[2];
    /** sad rate */
    TH1F* h_sad_sraw[2];
    /** sad s v E */
    TH2F* h_sad_sE[2];
    /** sad E */
    TH1F* h_sad_E[2];
    /** sad s */
    TH1F* h_sad_s[2];
    /** comp. E */
    TH1F* h_dE;
    /** comp. px */
    TH1F* h_dpx;
    /** comp. py */
    TH1F* h_dpy;
    /** E */
    TH1F* h_E;
    /** P */
    TH1F* h_P;
    /** px */
    TH1F* h_px;
    /** py */
    TH1F* h_py;
    /** pz */
    TH1F* h_pz;
    /** x */
    TH1F* h_dx;
    /** y */
    TH1F* h_dy;
    /** z */
    TH1F* h_dz;
    /** z */
    TH1F* h_z[2];

  };

  //  }
}

#endif /* ANALYSISPHASE1STUDYMODULE_H */
