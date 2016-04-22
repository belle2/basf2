/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Giulia Casarosa                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
/* Additional Info:
 * This Module is in an early stage of developement. The comments are mainly for temporal purposes
 * and will be changed and corrected in later stages of developement. So please ignore them.
 */

#ifndef TB_ANALYSIS_H_
#define TB_ANALYSIS_H_

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /** The TB Analysis Module
   *
   */

  class TBAnalysisModule : public Module, PerformanceEvaluationBaseClass {

  public:

    TBAnalysisModule();

    virtual ~TBAnalysisModule();
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:

    int m_goodL1inter;
    int m_goodL2inter;

    TH1F* m_h1_pValue; /**< pValue TH1F*/

    TH1F* m_h1_uResid_L1;
    TH1F* m_h1_vResid_L1;
    TH1F* m_h1_uResid_L2;
    TH1F* m_h1_vResid_L2;

    TH2F* m_h2_hitMap_vL1_uL1; /**< L1 hitmap*/
    TH2F* m_h2_hitMap_vL2_uL2; /**< L2 hitmap*/

    TH2F* m_h2_interMap_vL1_uL1; /**< L1 hitmap*/
    TH2F* m_h2_interMap_vL2_uL2; /**< L2 hitmap*/

    TH2F* m_h2_corr_vL3_vL1;
    TH2F* m_h2_corr_vL3_vL2;
    TH2F* m_h2_corr_uL3_uL1;
    TH2F* m_h2_corr_uL3_uL2;
    TH2F* m_h2_corr_uL4_uL1;
    TH2F* m_h2_corr_uL3_uL4;
    TH2F* m_h2_corr_vL3_uL1;
  };
}

#endif /* TBAnalysisModule_H_ */

