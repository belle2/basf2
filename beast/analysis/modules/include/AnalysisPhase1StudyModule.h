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
#include <framework/gearbox/Const.h>

#include <TH1.h>
#include <TH2.h>

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

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

    /**
     * Defines the histograms
     */
    virtual void defineHisto() override;

  private:

    /** counter */
    TH1F* h_count = nullptr;
    /** r v z */
    TH2F* h_prodvtx[10] = {nullptr};
    /** r v z */
    TH2F* h_decavtx[10] = {nullptr};
    /** kin v z */
    TH2F* h_kineticvz[10] = {nullptr};
    /** kin v z1 */
    TH2F* h_kineticvz1[10] = {nullptr};
    /** kin v z2 */
    TH2F* h_kineticvz2[10] = {nullptr};
    /** kin v z */
    TH2F* h_kineticvz_zoom[10] = {nullptr};
    /** kin v z */
    TH2F* h_Wkineticvz[10] = {nullptr};
    /** kin v z */
    TH2F* h_Wkineticvz_zoom[10] = {nullptr};
    /** theta v z */
    TH2F* h_thetavz[10] = {nullptr};
    /** phi v z */
    TH2F* h_phivz[10] = {nullptr};
    /** phi v e */
    TH2F* h_phive[10] = {nullptr};
    /** r v e */
    TH2F* h_rve[10] = {nullptr};
    /** sad xy */
    TH2F* h_sad_xy[2] = {nullptr};
    /** g4 xy */
    TH2F* h_g4_xy = nullptr;
    /** sad rate */
    TH1F* h_sad_sir[2] = {nullptr};
    /** sad rate */
    TH1F* h_sad_sall[2] = {nullptr};
    /** sad rate */
    TH1F* h_sad_sraw[2] = {nullptr};
    /** sad s v E */
    TH2F* h_sad_sE[2] = {nullptr};
    /** sad E */
    TH1F* h_sad_E[2] = {nullptr};
    /** sad s */
    TH1F* h_sad_s[2] = {nullptr};
    /** comp. E */
    TH1F* h_dE = nullptr;
    /** comp. px */
    TH1F* h_dpx = nullptr;
    /** comp. py */
    TH1F* h_dpy = nullptr;
    /** E */
    TH1F* h_E = nullptr;
    /** P */
    TH1F* h_P = nullptr;
    /** px */
    TH1F* h_px = nullptr;
    /** py */
    TH1F* h_py = nullptr;
    /** pz */
    TH1F* h_pz = nullptr;
    /** x */
    TH1F* h_dx = nullptr;
    /** y */
    TH1F* h_dy = nullptr;
    /** z */
    TH1F* h_dz = nullptr;
    /** z */
    TH1F* h_z[2] = {nullptr};

  };

  //  }
}

#endif /* ANALYSISPHASE1STUDYMODULE_H */
