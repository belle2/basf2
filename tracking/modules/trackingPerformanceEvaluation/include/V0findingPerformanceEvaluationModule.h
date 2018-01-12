/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef V0FINDINGPERFORMANCEEVALUATIONMODULE_H_
#define V0FINDINGPERFORMANCEEVALUATIONMODULE_H_

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TTree.h>
#include <TFile.h>
#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/MCParticleInfo.h>
#include <tracking/dataobjects/V0ValidationVertex.h>

// forward declarations
namespace Belle2 {

  class MCParticle;
  class V0;

  template< class T >
  class StoreArray;

  /** This module takes the MCParticles, the V0 candidates input and produce a root file
   *  containing various histograms showing the performance of the
   *  V0 finding module.
   */
  class V0findingPerformanceEvaluationModule : public Module, PerformanceEvaluationBaseClass {

  public:

    V0findingPerformanceEvaluationModule();

    ~V0findingPerformanceEvaluationModule();

    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    void terminate() override;

  private:

    bool isV0(const MCParticle& the_mcParticle); /**< is V0*/

    int nMatchedDaughters(const MCParticle& the_mcParticle); /**< number of truth matched dauhters*/

    /* user-defined parameters */
    std::string m_MCParticlesName; /**< MCParticle StoreArray name */
    std::string m_V0sName; /**< MCTrackCand StoreArray name */

    /* list of histograms filled per MCParticle found in the event */

    TH1F* m_multiplicityV0s; /**< number of V0s per MCParticles*/
    TH1F* m_multiplicityMCParticles;  /**< number of MCParticles per fitted Track*/
    TH1F* m_MCParticlePDGcode;  /**< MCParticle PDG code*/

    //error on track parameters
    TH1F* m_h1_vtxX_err; /**< vtx error*/
    TH1F* m_h1_vtxY_err; /**< vtx error*/
    TH1F* m_h1_vtxZ_err; /**< vtx error*/
    TH2F* m_h2_vtxTvsR_err; /**< vtx error on transverse plane VS transverse flight length*/
    //    TH1F* m_h1_mom_err; /**< mom error*/
    //    TH1F* m_h1_mass_err; /**< mom error*/
    //residuals on track parameters
    TH1F* m_h1_vtxX_res; /**< vtx resid*/
    TH1F* m_h1_vtxY_res; /**< vtx resid*/
    TH1F* m_h1_vtxZ_res; /**< vtx resid*/
    TH1F* m_h1_mom_res; /**< mom resid*/
    TH2F* m_h2_mom; /**< mom reco VS true*/
    TH1F* m_h1_mass_res; /**< mom resid*/
    TH2F* m_h2_mass; /**< mass reco VS true*/
    //pulls on track parameters
    TH1F* m_h1_vtxX_pll; /**< vtx pull*/
    TH1F* m_h1_vtxY_pll; /**< vtx pull*/
    TH1F* m_h1_vtxZ_pll; /**< vtx pull*/
    //    TH1F* m_h1_mom_pll; /**< mom pull*/
    //    TH1F* m_h1_mass_pll; /**< mom pull*/

    TH1F* m_h1_ChiSquare; /**< TH1F chi square */

    TH1F* m_h1_nMatchedDau; /**< TH1F n matched daughters*/

    //histograms used for efficiency plots
    TH3F* m_h3_MCParticle; /**< */
    TH3F* m_h3_V0sPerMCParticle; /**< */
    TH1F* m_h1_MCParticle_R; /**< */
    TH1F* m_h1_V0sPerMCParticle_R; /**< */

    //histograms used for purity plots
    TH3F* m_h3_V0s; /**< */
    TH3F* m_h3_MCParticlesPerV0; /**< */

  };
} // end of namespace


#endif /* V0DFINDINGPERFORMANCEEVALUAITONMODULE_H_ */
