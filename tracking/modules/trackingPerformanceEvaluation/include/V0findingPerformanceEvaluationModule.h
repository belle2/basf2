/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/modules/trackingPerformanceEvaluation/PerformanceEvaluationBaseClass.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>

// forward declarations
namespace Belle2 {

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

    TH1F* m_multiplicityV0s = nullptr; /**< number of V0s per MCParticles*/
    TH1F* m_multiplicityMCParticles = nullptr;  /**< number of MCParticles per fitted Track*/
    TH1F* m_MCParticlePDGcode = nullptr;  /**< MCParticle PDG code*/

    //error on track parameters
    TH1F* m_h1_vtxX_err = nullptr; /**< vtx error*/
    TH1F* m_h1_vtxY_err = nullptr; /**< vtx error*/
    TH1F* m_h1_vtxZ_err = nullptr; /**< vtx error*/
    TH2F* m_h2_vtxTvsR_err = nullptr; /**< vtx error on transverse plane VS transverse flight length*/
    //    TH1F* m_h1_mom_err; /**< mom error*/
    //    TH1F* m_h1_mass_err; /**< mom error*/
    //residuals on track parameters
    TH1F* m_h1_vtxX_res = nullptr; /**< vtx resid*/
    TH1F* m_h1_vtxY_res = nullptr; /**< vtx resid*/
    TH1F* m_h1_vtxZ_res = nullptr; /**< vtx resid*/
    TH1F* m_h1_mom_res = nullptr; /**< mom resid*/
    TH2F* m_h2_mom = nullptr; /**< mom reco VS true*/
    TH1F* m_h1_mass_res = nullptr; /**< mom resid*/
    TH2F* m_h2_mass = nullptr; /**< mass reco VS true*/
    //pulls on track parameters
    TH1F* m_h1_vtxX_pll = nullptr; /**< vtx pull*/
    TH1F* m_h1_vtxY_pll = nullptr; /**< vtx pull*/
    TH1F* m_h1_vtxZ_pll = nullptr; /**< vtx pull*/
    //    TH1F* m_h1_mom_pll; /**< mom pull*/
    //    TH1F* m_h1_mass_pll; /**< mom pull*/

    TH1F* m_h1_ChiSquare = nullptr; /**< TH1F chi square */

    TH1F* m_h1_nMatchedDau = nullptr; /**< TH1F n matched daughters*/

    //histograms used for efficiency plots
    TH3F* m_h3_MCParticle = nullptr; /**< V0-finding denominator */
    TH3F* m_h3_V0sPerMCParticle = nullptr; /**< V0-finding numerator */
    TH1F* m_h1_MCParticle_R = nullptr; /**< V0-finding denominator by radius */
    TH1F* m_h1_V0sPerMCParticle_R = nullptr; /**< V0-finding numerator by radius */

    //histograms used for purity plots
    TH3F* m_h3_V0s = nullptr; /**< V0-finding purity denominator */
    TH3F* m_h3_MCParticlesPerV0 = nullptr; /**< V0-finding numerator */

  };
} // end of namespace
