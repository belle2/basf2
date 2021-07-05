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

#include <TList.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>

#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {

  /** This module takes the MCParticles, the RecoTracks and Tracks/V0 in input and produce a root file
   *  containing various histograms showing the efficiencies (as a function of different variables) of the
   *  V0 finding module. Efficiencies normalized to MCParticles and RecoTracks are both produced.
   *  It requires V0ValidationVertexs.
   */

  class EffPlotsModule : public Module, PerformanceEvaluationBaseClass {

  public:

    EffPlotsModule();

    ~EffPlotsModule();

    void initialize() override;

    void beginRun() override;

    void event() override;

    void endRun() override;

    void terminate() override;

  private:

    /// determine if the MCParticle is a K-short
    bool isK_Short(const MCParticle& the_mcParticle);

    /// determine if the MCParticle is a Lambda0
    bool isLambda0(const MCParticle& the_mcParticle);

    /// get the number of matched daughters of the MCParticle
    int nMatchedDaughters(const MCParticle& the_mcParticle);

    /** user-defined parameters */
    std::string m_MCParticlesName; /**< name of the MCParticles dataobjects collection */
    std::string m_V0sName; /**< name of the V0s dataobjects collection */
    std::string m_RecoTracksName; /**< name of the RecoTracks dataobjects collection */
    std::string m_MCRecoTracksName; /**< name of the MCRecoTracks dataobjects collection */
    std::string m_TFRColName; /**< name of the TFR dataobjects collection */
    std::string m_TrackColName; /**< name of the Tracks dataobjects collection */
    std::string m_V0sType; /**< type (as a string) of the selected V0 */

    bool m_allHistograms; /**< true to create all histograms */
    bool m_geometricalAccettance; /**< true to create output for the geometrical acceptance */

    MCParticle* mc_dau0 = nullptr; /**< daughter 0 of a decayed MCParticle */
    MCParticle* mc_dau1 = nullptr; /**< daughter 1 of a decayed MCParticle */

    TList* m_histoList_MCParticles = nullptr; /**< list of histograms for MCParticles */
    TList* m_histoList_RecoTracks = nullptr; /**< list of histograms for RecoTracks */
    TList* m_histoList_Tracks = nullptr; /**< list of histograms for Tracks */
    TList* m_histoList_Efficiencies = nullptr; /**< list of histograms of efficiencies */
    TList* m_histoList_GA = nullptr; /**< list of histograms of geometric acceptance */
    TList* m_histoList_check = nullptr; /**< list of histograms of MCParticle mother and daughters */

    /** list of histograms filled per MCParticle found in the event */
    TH1F* m_h1_MC_dau0_d0 = nullptr; /**< histogram of MCParticle daughter 0's d0 */
    TH1F* m_h1_MC_dau0_z0 = nullptr; /**< histogram of MCParticle daughter 0's z0 */
    TH1F* m_h1_MC_dau0_RMother = nullptr; /**< histogram of MCParticle daughter 0's RMother */
    TH3F* m_h3_MC_dau0 = nullptr;  /**< histogram of MCParticle daughter 0's pt vs theta vs phi */
    TH1F* m_h1_MC_dau0_pt = nullptr;  /**< histogram of MCParticle daughter 0's pt */
    TH1F* m_h1_MC_dau0_pz = nullptr;  /**< histogram of MCParticle daughter 0's pz */
    TH1F* m_h1_MC_dau0_p = nullptr;  /**< histogram of MCParticle daughter 0's p */
    TH1F* m_h1_MC_dau0_phi = nullptr;  /**< histogram of MCParticle daughter 0's phi */
    TH1F* m_h1_MC_dau0_phi_BW = nullptr;  /**< histogram of MCParticle daughter 0's phi (backward region) */
    TH1F* m_h1_MC_dau0_phi_barrel = nullptr;  /**< histogram of MCParticle daughter 0's phi (barrel region) */
    TH1F* m_h1_MC_dau0_phi_FW = nullptr;  /**< histogram of MCParticle daughter 0's phi (forward region) */
    TH1F* m_h1_MC_dau0_theta = nullptr;  /**< histogram of MCParticle daughter 0's theta */
    TH1F* m_h1_MC_dau0_costheta = nullptr;  /**< histogram of MCParticle daughter 0's cos(theta) */
    TH1F* m_h1_MC_dau0_Mother_cosAngle = nullptr;  /**< histogram of MCParticle daughter 0's and mother's cos(opening-angle) */
    TH1F* m_h1_MC_dau0_phiMother_total = nullptr;  /**< histogram of MCParticle daughter 0's mother's phi */
    TH1F* m_h1_MC_dau0_phiMother_BW = nullptr;  /**< histogram of MCParticle daughter 0's mother's phi (backward region) */
    TH1F* m_h1_MC_dau0_phiMother_barrel = nullptr;  /**< histogram of MCParticle daughter 0's mother's phi (barrel region) */
    TH1F* m_h1_MC_dau0_phiMother_FW = nullptr;  /**< histogram of MCParticle daughter 0's mother's phi (forward region) */
    TH1F* m_h1_MC_dau0_thetaMother = nullptr;  /**< histogram of MCParticle daughter 0's mother's theta */
    TH1F* m_h1_MC_dau0_ptMother = nullptr;  /**< histogram of MCParticle daughter 0's mother's pt */
    TH2F* m_h2_MC_dau0_2D = nullptr;  /**< histogram of MCParticle daughter 0's pt vs theta */
    TH2F* m_h2_MC_dau0_2D_BP = nullptr;  /**< histogram of MCParticle daughter 0's pt vs theta (beam pipe) */
    TH2F* m_h2_MC_dau0_2DMother = nullptr;  /**< histogram of MCParticle daughter 0's mother's pt vs theta */
    TH2F* m_h2_MC_dau0_pVScostheta = nullptr;  /**< histogram of MCParticle daughter 0's p vs cos(theta) */
    TH1F* m_h1_MC_dau0_PDG = nullptr;  /**< histogram of MCParticle daughter 0's PDG code */

    TH1F* m_h1_MC_dau1_d0 = nullptr; /**< histogram of MCParticle daughter 1's d0 */
    TH1F* m_h1_MC_dau1_z0 = nullptr; /**< histogram of MCParticle daughter 1's z0 */
    TH1F* m_h1_MC_dau1_RMother = nullptr; /**< histogram of MCParticle daughter 1's RMother */
    TH3F* m_h3_MC_dau1 = nullptr; /**< histogram of MCParticle daughter 1's pt vs theta vs phi */
    TH1F* m_h1_MC_dau1_pt = nullptr; /**< histogram of MCParticle daughter 1's pt */
    TH1F* m_h1_MC_dau1_pz = nullptr; /**< histogram of MCParticle daughter 1's pz */
    TH1F* m_h1_MC_dau1_p = nullptr; /**< histogram of MCParticle daughter 1's p */
    TH1F* m_h1_MC_dau1_phi = nullptr; /**< histogram of MCParticle daughter 1's phi */
    TH1F* m_h1_MC_dau1_phi_BW = nullptr;  /**< histogram of MCParticle daughter 1's phi (backward region) */
    TH1F* m_h1_MC_dau1_phi_barrel = nullptr; /**< histogram of MCParticle daughter 1's phi (barrel region) */
    TH1F* m_h1_MC_dau1_phi_FW = nullptr; /**< histogram of MCParticle daughter 1's phi (forward region) */
    TH1F* m_h1_MC_dau1_theta = nullptr; /**< histogram of MCParticle daughter 1's theta */
    TH1F* m_h1_MC_dau1_costheta = nullptr; /**< histogram of MCParticle daughter 1's cos(theta) */
    TH1F* m_h1_MC_dau1_Mother_cosAngle = nullptr; /**< histogram of MCParticle daughter 1's and mother's cos(opening-angle) */
    TH1F* m_h1_MC_dau1_phiMother_total = nullptr; /**< histogram of MCParticle daughter 1's mother's phi */
    TH1F* m_h1_MC_dau1_phiMother_BW = nullptr; /**< histogram of MCParticle daughter 1's mother's phi (backward region) */
    TH1F* m_h1_MC_dau1_phiMother_barrel = nullptr; /**< histogram of MCParticle daughter 1's mother's phi (barrel region) */
    TH1F* m_h1_MC_dau1_phiMother_FW = nullptr; /**< histogram of MCParticle daughter 1's mother's phi (forward region) */
    TH1F* m_h1_MC_dau1_thetaMother = nullptr; /**< histogram of MCParticle daughter 1's mother's theta */
    TH1F* m_h1_MC_dau1_ptMother = nullptr; /**< histogram of MCParticle daughter 1's mother's pt */
    TH2F* m_h2_MC_dau1_2D = nullptr; /**< histogram of MCParticle daughter 1's pt vs theta */
    TH2F* m_h2_MC_dau1_2D_BP = nullptr; /**< histogram of MCParticle daughter 1's pt vs theta (beam pipe) */
    TH2F* m_h2_MC_dau1_2DMother = nullptr; /**< histogram of MCParticle daughter 1's mother's pt vs theta */
    TH2F* m_h2_MC_dau1_pVScostheta = nullptr; /**< histogram of MCParticle daughter 1's p vs cos(theta) */
    TH1F* m_h1_MC_dau1_PDG = nullptr; /**< histogram of MCParticle daughter 1's PDG code */

    TH1F* m_h1_MC_Mother_RMother = nullptr; /**< histogram of MCParticle mother's RMother */
    TH3F* m_h3_MC_Mother = nullptr; /**< histogram of MCParticle mother's pt vs theta vs phi */
    TH1F* m_h1_MC_Mother_pt = nullptr; /**< histogram of MCParticle mother's pt */
    TH1F* m_h1_MC_Mother_pz = nullptr; /**< histogram of MCParticle mother's pz */
    TH1F* m_h1_MC_Mother_p = nullptr; /**< histogram of MCParticle mother's p */
    TH1F* m_h1_MC_Mother_phi = nullptr; /**< histogram of MCParticle mother's phi */
    TH1F* m_h1_MC_Mother_phi_BW = nullptr; /**< histogram of MCParticle mother's phi (backward region) */
    TH1F* m_h1_MC_Mother_phi_barrel = nullptr; /**< histogram of MCParticle mother's phi (barrel region) */
    TH1F* m_h1_MC_Mother_phi_FW = nullptr; /**< histogram of MCParticle mother's phi (forward region) */
    TH1F* m_h1_MC_Mother_theta = nullptr; /**< histogram of MCParticle mother's theta */
    TH1F* m_h1_MC_Mother_costheta = nullptr; /**< histogram of MCParticle mother's cos(theta) */
    TH2F* m_h2_MC_Mother_2D = nullptr; /**< histogram of MCParticle mother's pt vs theta */
    TH2F* m_h2_MC_Mother_2D_BP = nullptr; /**< histogram of MCParticle mother's pt vs theta (beam pipe) */
    TH2F* m_h2_MC_Mother_pVScostheta = nullptr; /**< histogram of MCParticle mother's p vs cos(theta) */
    TH1F* m_h1_MC_Mother_PDG = nullptr; /**< histogram of MCParticle mother's PDG code */

    /** list of histograms filled per Tracks/V0 found in the event */
    TH1F* m_h1_track_dau0_d0 = nullptr; /**< histogram of Track daughter 0's d0 */
    TH1F* m_h1_track_dau0_z0 = nullptr; /**< histogram of Track daughter 0's z0 */
    TH1F* m_h1_track_dau0_RMother = nullptr; /**< histogram of Track daughter 0's RMother */
    TH3F* m_h3_track_dau0 = nullptr; /**< histogram of Track daughter 0's pt vs theta vs phi */
    TH1F* m_h1_track_dau0_pt = nullptr;  /**< histogram of Track daughter 0's pt */
    TH1F* m_h1_track_dau0_pz = nullptr; /**< histogram of Track daughter 0's pz */
    TH1F* m_h1_track_dau0_p = nullptr; /**< histogram of Track daughter 0's p */
    TH1F* m_h1_track_dau0_phi = nullptr; /**< histogram of Track daughter 0's phi */
    TH1F* m_h1_track_dau0_phi_BW = nullptr; /**< histogram of Track daughter 0's phi (backward region) */
    TH1F* m_h1_track_dau0_phi_barrel = nullptr; /**< histogram of Track daughter 0's phi (barrel region) */
    TH1F* m_h1_track_dau0_phi_FW = nullptr; /**< histogram of Track daughter 0's phi (forward region) */
    TH1F* m_h1_track_dau0_theta = nullptr; /**< histogram of Track daughter 0's theta */
    TH1F* m_h1_track_dau0_costheta = nullptr; /**< histogram of Track daughter 0's cos(theta) */
    TH1F* m_h1_track_dau0_Mother_cosAngle = nullptr; /**< histogram of Track daughter 0's and mother's cos(opening-angle) */
    TH1F* m_h1_track_dau0_phiMother_total = nullptr; /**< histogram of Track daughter 0's mother's phi */
    TH1F* m_h1_track_dau0_phiMother_BW = nullptr; /**< histogram of Track daughter 0's mother's phi (backward region) */
    TH1F* m_h1_track_dau0_phiMother_barrel = nullptr; /**< histogram of Track daughter 0's mother's phi (barrel region) */
    TH1F* m_h1_track_dau0_phiMother_FW = nullptr; /**< histogram of Track daughter 0's mother's phi (forward region) */
    TH1F* m_h1_track_dau0_thetaMother = nullptr; /**< histogram of Track daughter 0's mother's theta */
    TH1F* m_h1_track_dau0_ptMother = nullptr; /**< histogram of Track daughter 0's mother's pt */
    TH2F* m_h2_track_dau0_2D = nullptr; /**< histogram of Track daughter 0's pt vs theta */
    TH2F* m_h2_track_dau0_2D_BP = nullptr; /**< histogram of Track daughter 0's pt vs theta (beam pipe) */
    TH2F* m_h2_track_dau0_2DMother = nullptr; /**< histogram of Track daughter 0's mother's pt vs theta */
    TH2F* m_h2_track_dau0_pVScostheta = nullptr; /**< histogram of Track daughter 0's p vs cos(theta) */

    TH1F* m_h1_track_dau1_d0 = nullptr; /**< histogram of Track daughter 1's d0 */
    TH1F* m_h1_track_dau1_z0 = nullptr; /**< histogram of Track daughter 1's z0 */
    TH1F* m_h1_track_dau1_RMother = nullptr; /**< histogram of Track daughter 1's RMother */
    TH3F* m_h3_track_dau1 = nullptr; /**< histogram of Track daughter 1's pt vs theta vs phi */
    TH1F* m_h1_track_dau1_pt = nullptr; /**< histogram of Track daughter 1's pt */
    TH1F* m_h1_track_dau1_pz = nullptr; /**< histogram of Track daughter 1's pz */
    TH1F* m_h1_track_dau1_p = nullptr; /**< histogram of Track daughter 1's p */
    TH1F* m_h1_track_dau1_phi = nullptr; /**< histogram of Track daughter 1's phi */
    TH1F* m_h1_track_dau1_phi_BW = nullptr; /**< histogram of Track daughter 1's phi (backward region) */
    TH1F* m_h1_track_dau1_phi_barrel = nullptr; /**< histogram of Track daughter 1's phi (barrel region) */
    TH1F* m_h1_track_dau1_phi_FW = nullptr; /**< histogram of Track daughter 1's phi (forward region) */
    TH1F* m_h1_track_dau1_theta = nullptr; /**< histogram of Track daughter 1's theta */
    TH1F* m_h1_track_dau1_costheta = nullptr; /**< histogram of Track daughter 1's cos(theta) */
    TH1F* m_h1_track_dau1_Mother_cosAngle = nullptr; /**< histogram of Track daughter 1's and mother's cos(opening-angle) */
    TH1F* m_h1_track_dau1_phiMother_total = nullptr; /**< histogram of Track daughter 1's mother's phi */
    TH1F* m_h1_track_dau1_phiMother_BW = nullptr; /**< histogram of Track daughter 1's mother's phi (backward region) */
    TH1F* m_h1_track_dau1_phiMother_barrel = nullptr; /**< histogram of Track daughter 1's mother's phi (barrel region) */
    TH1F* m_h1_track_dau1_phiMother_FW = nullptr; /**< histogram of Track daughter 1's mother's phi (forward region) */
    TH1F* m_h1_track_dau1_thetaMother = nullptr; /**< histogram of Track daughter 1's mother's theta */
    TH1F* m_h1_track_dau1_ptMother = nullptr; /**< histogram of Track daughter 1's mother's pt */
    TH2F* m_h2_track_dau1_2D = nullptr; /**< histogram of Track daughter 1's pt vs theta */
    TH2F* m_h2_track_dau1_2D_BP = nullptr; /**< histogram of Track daughter 1's pt vs theta (beam pipe) */
    TH2F* m_h2_track_dau1_2DMother = nullptr; /**< histogram of Track daughter 1's mother's pt vs theta */
    TH2F* m_h2_track_dau1_pVScostheta = nullptr; /**< histogram of Track daughter 1's p vs cos(theta) */

    TH1F* m_h1_V0_RMother = nullptr; /**< histogram of V0 mother's RMother */
    TH3F* m_h3_V0 = nullptr; /**< histogram of V0 mother's pt vs theta vs phi */
    TH1F* m_h1_V0_pt = nullptr; /**< histogram of V0 mother's pt */
    TH1F* m_h1_V0_pz = nullptr; /**< histogram of V0 mother's pz */
    TH1F* m_h1_V0_p = nullptr; /**< histogram of V0 mother's p */
    TH1F* m_h1_V0_phi = nullptr; /**< histogram of V0 mother's phi */
    TH1F* m_h1_V0_phi_BW = nullptr; /**< histogram of V0 mother's phi (backward region) */
    TH1F* m_h1_V0_phi_barrel = nullptr; /**< histogram of V0 mother's phi (barrel region) */
    TH1F* m_h1_V0_phi_FW = nullptr; /**< histogram of V0 mother's phi (forward region) */
    TH1F* m_h1_V0_theta = nullptr; /**< histogram of V0 mother's theta */
    TH1F* m_h1_V0_costheta = nullptr; /**< histogram of V0 mother's cos(theta) */
    TH2F* m_h2_V0_Mother_2D = nullptr; /**< histogram of V0 mother's pt vs theta */
    TH2F* m_h2_V0_Mother_2D_BP = nullptr; /**< histogram of V0 mother's pt vs theta (beam pipe) */
    TH2F* m_h2_V0_Mother_pVScostheta = nullptr; /**< histogram of V0 mother's p vs cos(theta) */

    /** list of histograms filled per RecoTracks found in the event */
    TH1F* m_h1_RecoTrack_dau0_d0 = nullptr; /**< histogram of RecoTrack daughter 0's d0 */
    TH1F* m_h1_RecoTrack_dau0_z0 = nullptr; /**< histogram of RecoTrack daughter 0's z0 */
    TH1F* m_h1_RecoTrack_dau0_RMother = nullptr; /**< histogram of RecoTrack daughter 0's RMother */
    TH3F* m_h3_RecoTrack_dau0 = nullptr; /**< histogram of RecoTrack daughter 0's pt vs theta vs phi */
    TH1F* m_h1_RecoTrack_dau0_pt = nullptr; /**< histogram of RecoTrack daughter 0's pt */
    TH1F* m_h1_RecoTrack_dau0_pz = nullptr;/**< histogram of RecoTrack daughter 0's pz */
    TH1F* m_h1_RecoTrack_dau0_p = nullptr; /**< histogram of RecoTrack daughter 0's p */
    TH1F* m_h1_RecoTrack_dau0_phi = nullptr; /**< histogram of RecoTrack daughter 0's phi */
    TH1F* m_h1_RecoTrack_dau0_phi_BW = nullptr; /**< histogram of RecoTrack daughter 0's phi (backward region) */
    TH1F* m_h1_RecoTrack_dau0_phi_barrel = nullptr; /**< histogram of RecoTrack daughter 0's phi (barrel region) */
    TH1F* m_h1_RecoTrack_dau0_phi_FW = nullptr; /**< histogram of RecoTrack daughter 0's phi (forward region) */
    TH1F* m_h1_RecoTrack_dau0_theta = nullptr; /**< histogram of RecoTrack daughter 0's theta */
    TH1F* m_h1_RecoTrack_dau0_costheta = nullptr; /**< histogram of RecoTrack daughter 0's cos(theta) */
    TH1F* m_h1_RecoTrack_dau0_Mother_cosAngle = nullptr; /**< histogram of RecoTrack daughter 0's and mother's cos(opening-angle) */
    TH1F* m_h1_RecoTrack_dau0_phiMother_total = nullptr; /**< histogram of RecoTrack daughter 0's mother's phi */
    TH1F* m_h1_RecoTrack_dau0_phiMother_BW = nullptr; /**< histogram of RecoTrack daughter 0's mother's phi (backward region) */
    TH1F* m_h1_RecoTrack_dau0_phiMother_barrel = nullptr; /**< histogram of RecoTrack daughter 0's mother's phi (barrel region) */
    TH1F* m_h1_RecoTrack_dau0_phiMother_FW = nullptr; /**< histogram of RecoTrack daughter 0's mother's phi (forward region) */
    TH1F* m_h1_RecoTrack_dau0_thetaMother = nullptr; /**< histogram of RecoTrack daughter 0's mother's theta */
    TH1F* m_h1_RecoTrack_dau0_ptMother = nullptr; /**< histogram of RecoTrack daughter 0's mother's pt */
    TH2F* m_h2_RecoTrack_dau0_2D = nullptr; /**< histogram of RecoTrack daughter 0's pt vs theta */
    TH2F* m_h2_RecoTrack_dau0_2D_BP = nullptr; /**< histogram of RecoTrack daughter 0's pt vs theta (beam pipe) */
    TH2F* m_h2_RecoTrack_dau0_2DMother = nullptr; /**< histogram of RecoTrack daughter 0's mother's pt vs theta */
    TH2F* m_h2_RecoTrack_dau0_pVScostheta = nullptr; /**< histogram of RecoTrack daughter 0's p vs cos(theta) */

    TH1F* m_h1_RecoTrack_dau1_d0 = nullptr; /**< histogram of RecoTrack daughter 1's d0 */
    TH1F* m_h1_RecoTrack_dau1_z0 = nullptr; /**< histogram of RecoTrack daughter 1's z0 */
    TH1F* m_h1_RecoTrack_dau1_RMother = nullptr; /**< histogram of RecoTrack daughter 1's RMother */
    TH3F* m_h3_RecoTrack_dau1 = nullptr; /**< histogram of RecoTrack daughter 1's pt vs theta vs phi */
    TH1F* m_h1_RecoTrack_dau1_pt = nullptr; /**< histogram of RecoTrack daughter 1's pt */
    TH1F* m_h1_RecoTrack_dau1_pz = nullptr; /**< histogram of RecoTrack daughter 1's pz */
    TH1F* m_h1_RecoTrack_dau1_p = nullptr; /**< histogram of RecoTrack daughter 1's p */
    TH1F* m_h1_RecoTrack_dau1_phi = nullptr; /**< histogram of RecoTrack daughter 1's phi */
    TH1F* m_h1_RecoTrack_dau1_phi_BW = nullptr; /**< histogram of RecoTrack daughter 1's phi (backward region) */
    TH1F* m_h1_RecoTrack_dau1_phi_barrel = nullptr; /**< histogram of RecoTrack daughter 1's phi (barrel region) */
    TH1F* m_h1_RecoTrack_dau1_phi_FW = nullptr; /**< histogram of RecoTrack daughter 1's phi (forward region) */
    TH1F* m_h1_RecoTrack_dau1_theta = nullptr; /**< histogram of RecoTrack daughter 1's theta */
    TH1F* m_h1_RecoTrack_dau1_costheta = nullptr; /**< histogram of RecoTrack daughter 1's cos(theta) */
    TH1F* m_h1_RecoTrack_dau1_Mother_cosAngle = nullptr; /**< histogram of RecoTrack daughter 1's and mother's cos(opening-angle) */
    TH1F* m_h1_RecoTrack_dau1_phiMother_total = nullptr; /**< histogram of RecoTrack daughter 1's mother's phi */
    TH1F* m_h1_RecoTrack_dau1_phiMother_BW = nullptr; /**< histogram of RecoTrack daughter 1's mother's phi (backward region) */
    TH1F* m_h1_RecoTrack_dau1_phiMother_barrel = nullptr; /**< histogram of RecoTrack daughter 1's mother's phi (barrel region) */
    TH1F* m_h1_RecoTrack_dau1_phiMother_FW = nullptr; /**< histogram of RecoTrack daughter 1's mother's phi (forward region) */
    TH1F* m_h1_RecoTrack_dau1_thetaMother = nullptr; /**< histogram of RecoTrack daughter 1's mother's theta */
    TH1F* m_h1_RecoTrack_dau1_ptMother = nullptr; /**< histogram of RecoTrack daughter 1's mother's pt */
    TH2F* m_h2_RecoTrack_dau1_2D = nullptr; /**< histogram of RecoTrack daughter 1's pt vs theta */
    TH2F* m_h2_RecoTrack_dau1_2D_BP = nullptr; /**< histogram of RecoTrack daughter 1's pt vs theta (beam pipe) */
    TH2F* m_h2_RecoTrack_dau1_2DMother = nullptr; /**< histogram of RecoTrack daughter 1's mother's pt vs theta */
    TH2F* m_h2_RecoTrack_dau1_pVScostheta = nullptr; /**< histogram of RecoTrack daughter 1's p vs cos(theta) */

    TH1F* m_h1_RecoTrack_Mother_RMother = nullptr; /**< histogram of RecoTrack mother's RMother */
    TH3F* m_h3_RecoTrack_Mother = nullptr; /**< histogram of RecoTrack mother's pt vs theta vs phi */
    TH1F* m_h1_RecoTrack_Mother_pt = nullptr; /**< histogram of RecoTrack mother's pt */
    TH1F* m_h1_RecoTrack_Mother_pz = nullptr; /**< histogram of RecoTrack mother's pz */
    TH1F* m_h1_RecoTrack_Mother_p = nullptr; /**< histogram of RecoTrack mother's p */
    TH1F* m_h1_RecoTrack_Mother_phi = nullptr; /**< histogram of RecoTrack mother's phi */
    TH1F* m_h1_RecoTrack_Mother_phi_BW = nullptr; /**< histogram of RecoTrack mother's phi (backward region) */
    TH1F* m_h1_RecoTrack_Mother_phi_barrel = nullptr; /**< histogram of RecoTrack mother's phi (barrel region) */
    TH1F* m_h1_RecoTrack_Mother_phi_FW = nullptr; /**< histogram of RecoTrack mother's phi (forward region) */
    TH1F* m_h1_RecoTrack_Mother_theta = nullptr; /**< histogram of RecoTrack mother's theta */
    TH1F* m_h1_RecoTrack_Mother_costheta = nullptr; /**< histogram of RecoTrack mother's cos(theta) */
    TH2F* m_h2_RecoTrack_Mother_2D = nullptr; /**< histogram of RecoTrack mother's pt vs theta */
    TH2F* m_h2_RecoTrack_Mother_2D_BP = nullptr; /**< histogram of RecoTrack mother's pt vs theta (beam pipe) */
    TH2F* m_h2_RecoTrack_Mother_pVScostheta = nullptr; /**< histogram of RecoTrack mother's p vs cos(theta) */

  };
} // end of namespace
