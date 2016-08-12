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
    virtual void initialize();

    /**  */
    virtual void beginRun();

    /**  */
    virtual void event();

    /**  */
    virtual void endRun();

    /**  */
    virtual void terminate();

    /** Defines the histograms*/
    virtual void defineHisto();

  private:

    /** counter */
    TH1F* h_count;
    /** r v z */
    TH2F* h_prodvtx[10];
    /** r v z */
    TH2F* h_decavtx[10];
    /** kin v z */
    TH2F* h_kineticvz[10];
    /** kin v z */
    TH2F* h_kineticvz_zoom[10];
    /** theta v z */
    TH2F* h_thetavz[10];
    /** phi v z */
    TH2F* h_phivz[10];
  };

  //  }
}

#endif /* ANALYSISPHASE1STUDYMODULE_H */
