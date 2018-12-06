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

    ~TBAnalysisModule();
    void initialize() override;
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

    int m_TrigDiv; /**< */
    std::string m_TrackCandName; /**< */
    std::string m_TrackName; /**< */
    std::string m_TrackFitResultName; /**< */
    std::string m_PXDInterceptName; /**< */
    std::string m_ROIidName; /**< */

  private:

    int m_goodL1inter; /**< good intercept */
    int m_goodL2inter; /**< good intercept */

    TH1F* m_h1_pValue; /**< pValue TH1F*/
    TH1F* m_h1_mom; /**< pValue TH1F*/

    TH1F* m_h1_nROIs_odd; /**< n ROIs */
    TH1F* m_h1_nROIs_even; /**< n ROIs */

    TH1F* m_h1_uwidthROI_odd; /**< ROI width */
    TH1F* m_h1_vwidthROI_odd; /**< ROI width */
    TH1F* m_h1_uwidthROI_even; /**< ROI width */
    TH1F* m_h1_vwidthROI_even; /**< ROI width */

    TH2F* m_h2_uvResidCluster_L1; /**< residuals */
    TH2F* m_h2_uvResidCluster_L2; /**< residuals */
    TH2F* m_h2_uvResidDigit_L1; /**< residuals */
    TH2F* m_h2_uvResidDigit_L2; /**< residuals*/


    //    TH2F*  m_h2_PXDCluster_L1;
    //    TH2F*  m_h2_PXDCluster_L2;
    TH2F*  m_h2_Intercepts_L1; /**< intecepts */
    TH2F*  m_h2_Intercepts_L2; /**< intercepts */

    TH1F* m_h1_uResid_L1_odd; /**< residuals */
    TH1F* m_h1_vResid_L1_odd; /**< residuals */
    TH1F* m_h1_uResid_L2_odd; /**< residuals */
    TH1F* m_h1_vResid_L2_odd; /**< residuals */
    TH1F* m_h1_uResid_L1_even; /**< residuals */
    TH1F* m_h1_vResid_L1_even; /**< residuals */
    TH1F* m_h1_uResid_L2_even; /**< residuals */
    TH1F* m_h1_vResid_L2_even; /**< residuals */
    TH1F* m_h1_uResid_L2_future_even; /**< residuals */
    TH1F* m_h1_vResid_L2_future_even; /**< residuals */

    TH2F* m_h2_hitMap_vL1_uL1; /**< L1 hitmap*/
    TH2F* m_h2_hitMap_vL2_uL2; /**< L2 hitmap*/

    TH2F* m_h2_interMap_vL1_uL1; /**< L1 hitmap*/
    TH2F* m_h2_interMap_vL2_uL2; /**< L2 hitmap*/

    TH2F* m_h2_corr_vL3_vL1; /**< correlations */
    TH2F* m_h2_corr_vL3_vL2; /**< correlations */
    TH2F* m_h2_corr_uL3_uL1_even; /**< correlations */
    TH2F* m_h2_corr_uL3_uL2_even; /**< correlations */
    TH2F* m_h2_corr_uL3_uL1_odd; /**< correlations */
    TH2F* m_h2_corr_uL3_uL2_odd;/**< correlations */
    TH2F* m_h2_corr_uL4_uL1; /**< correlations */
    TH2F* m_h2_corr_uL3_uL4; /**< correlations */
    TH2F* m_h2_corr_vL3_vL4; /**< correlations */
    TH2F* m_h2_corr_vL3_uL1; /**< correlations */
    //    TH2F* m_h2_corr_uL1_uL2;
    //    TH2F* m_h2_corr_vL1_vL2;
  };
}

#endif /* TBAnalysisModule_H_ */

