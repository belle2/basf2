/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>

#include <mdst/dataobjects/TRGSummary.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class TRGEFFDQMModule : public HistoModule {

  public:

    /**
     * Constructor
     */
    TRGEFFDQMModule();

    /**
     * Destructor
     */
    virtual ~TRGEFFDQMModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override;

    /**
     * Histogram definitions.
     *
     */
    void defineHisto() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    void beginRun() override;

    /**
     * Event processor.
     */
    void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override;


  protected:

    StoreObjPtr<TRGSummary> m_trgSummary;         /**< Trigger summary. */
    StoreArray<Track> m_Tracks;                   /**< Tracks. */
    StoreArray<ECLCluster>  m_ECLClusters;        /**< ECL Clusters. */
    StoreArray<KLMCluster>  m_KLMClusters;        /**< KLM Clusters. */
    StoreArray<RecoTrack> m_RecoTracks;           /**< RecoTracks. */
    std::string m_recoTrackArrayName = "" ;       /**< Belle2::RecoTrack StoreArray name. */
    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */

    TH1F* m_hPhi                = nullptr;            /**< Histogram of cdc phi of IP tracks*/
    TH1F* m_hPhi_psnecl         = nullptr;            /**< Histogram of cdc phi of IP tracks with ecl bit*/
    TH1F* m_hPhi_psnecl_ftdf    = nullptr;            /**< Histogram of cdc phi of IP tracks with ecl and f bit*/

    TH1F* m_hPt                 = nullptr;
    TH1F* m_hPt_psnecl          = nullptr;
    TH1F* m_hPt_psnecl_ftdf     = nullptr;

    TH1F* m_nobha_hPt             = nullptr; // remove the bhabha veto
    TH1F* m_nobha_hPt_psnecl      = nullptr; // remove the bhabha veto
    TH1F* m_nobha_hPt_psnecl_ftdf = nullptr; // remove the bhabha veto

    //for z bit (t3>0 and !bhaveto and !veto)
    TH1F* m_hP3_z                 = nullptr;
    TH1F* m_hP3_z_psnecl          = nullptr;
    TH1F* m_hP3_z_psnecl_ftdf     = nullptr;

    //for y bit (t3>0and!bhaveto and !veto)
    TH1F* m_hP3_y                 = nullptr;
    TH1F* m_hP3_y_psnecl          = nullptr;
    TH1F* m_hP3_y_psnecl_ftdf     = nullptr;

    //remove bhabha veto
    TH1F* m_nobha_hP3_z             = nullptr;
    TH1F* m_nobha_hP3_z_psnecl      = nullptr;
    TH1F* m_nobha_hP3_z_psnecl_ftdf = nullptr;

    TH1F* m_nobha_hP3_y             = nullptr;
    TH1F* m_nobha_hP3_y_psnecl      = nullptr;
    TH1F* m_nobha_hP3_y_psnecl_ftdf = nullptr;


    //-"fyo" with track deleta_phi distribution
    TH1F* m_fyo_dphi                    = nullptr;
    TH1F* m_fyo_dphi_psnecl             = nullptr;
    TH1F* m_fyo_dphi_psnecl_ftdf        = nullptr;

    TH1F* m_nobha_fyo_dphi              = nullptr;
    TH1F* m_nobha_fyo_dphi_psnecl       = nullptr;
    TH1F* m_nobha_fyo_dphi_psnecl_ftdf  = nullptr;

    //-"stt " with track p, θ, phi distribution
    TH1F* m_stt_phi                     = nullptr;
    TH1F* m_stt_phi_psnecl              = nullptr;
    TH1F* m_stt_phi_psnecl_ftdf         = nullptr;

    TH1F* m_stt_P3                      = nullptr;
    TH1F* m_stt_P3_psnecl               = nullptr;
    TH1F* m_stt_P3_psnecl_ftdf          = nullptr;

    TH1F* m_stt_theta                   = nullptr;
    TH1F* m_stt_theta_psnecl            = nullptr;
    TH1F* m_stt_theta_psnecl_ftdf       = nullptr;

    TH1F* m_nobha_stt_phi               = nullptr;
    TH1F* m_nobha_stt_phi_psnecl        = nullptr;
    TH1F* m_nobha_stt_phi_psnecl_ftdf   = nullptr;

    TH1F* m_nobha_stt_P3                = nullptr;
    TH1F* m_nobha_stt_P3_psnecl         = nullptr;
    TH1F* m_nobha_stt_P3_psnecl_ftdf    = nullptr;

    TH1F* m_nobha_stt_theta             = nullptr;
    TH1F* m_nobha_stt_theta_psnecl      = nullptr;
    TH1F* m_nobha_stt_theta_psnecl_ftdf = nullptr;

    //-"hie" with ECL cluster energy sum distribution.
    TH1F* m_hie_E                       = nullptr;
    TH1F* m_hie_E_psnecl                = nullptr;
    TH1F* m_hie_E_psnecl_ftdf           = nullptr;

    TH1F* m_nobha_hie_E                 = nullptr;    // remove the bha_veto
    TH1F* m_nobha_hie_E_psnecl          = nullptr;
    TH1F* m_nobha_hie_E_psnecl_ftdf     = nullptr;

    // ecltiming
    TH1F* m_ecltiming_E                 = nullptr;
    TH1F* m_ecltiming_E_psnecl          = nullptr;
    TH1F* m_ecltiming_E_psnecl_ftdf     = nullptr;

    TH1F* m_ecltiming_theta             = nullptr;
    TH1F* m_ecltiming_theta_psnecl      = nullptr;
    TH1F* m_ecltiming_theta_psnecl_ftdf = nullptr;

    TH1F* m_ecltiming_phi               = nullptr;
    TH1F* m_ecltiming_phi_psnecl        = nullptr;
    TH1F* m_ecltiming_phi_psnecl_ftdf   = nullptr;

    // -KLM TRG - - - for track φ,θ distribution
    TH1F* m_klmhit_phi                = nullptr;
    TH1F* m_klmhit_phi_psnecl         = nullptr;
    TH1F* m_klmhit_phi_psnecl_ftdf    = nullptr;

    TH1F* m_klmhit_theta              = nullptr;
    TH1F* m_klmhit_theta_psnecl       = nullptr;
    TH1F* m_klmhit_theta_psnecl_ftdf  = nullptr;

    TH1F* m_eklmhit_phi               = nullptr;
    TH1F* m_eklmhit_phi_psnecl        = nullptr;
    TH1F* m_eklmhit_phi_psnecl_ftdf   = nullptr;

    TH1F* m_eklmhit_theta             = nullptr;
    TH1F* m_eklmhit_theta_psnecl      = nullptr;
    TH1F* m_eklmhit_theta_psnecl_ftdf = nullptr;












  };

} // Belle2 namespace
