/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRCGRLDQMMODULE_h
#define TRCGRLDQMMODULE_h

#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>

class TH1I;
class TH1F;
class TH2F;

namespace Belle2 {

  class TRGGRLDQMModule : public HistoModule {

  public:
    //! Constructor
    TRGGRLDQMModule();
    //! Destrunctor
    virtual ~TRGGRLDQMModule() {}

  public:
    //! initialize
    virtual void initialize() override;
    //! begin Run
    virtual void beginRun() override;
    //! Event
    virtual void event() override;
    //! End Run
    virtual void endRun() override;
    //! terminate
    virtual void terminate() override {}
    //! Define Histogram
    virtual void defineHisto() override;

  protected:
    //! TDirectory
    TDirectory* oldDir = nullptr;
    //! TDirectory
    TDirectory* dirDQM = nullptr;

    /** Number of tracks */
    TH1I* h_N_track = nullptr;

    /** phi_i of CDC 2D tracks */
    TH1F* h_phi_i = nullptr;
    /** phi_CDC, extrapolated phi at ECL of CDC 2D tracks */
    TH1F* h_phi_CDC = nullptr;
    /** sector_CDC, extrapolated phi at KLM of CDC 2D tracks */
    TH1F* h_sector_CDC = nullptr;
    /** sector map of KLM */
    TH1F* h_sector_KLM = nullptr;
    /** slot_CDC, extrapolated phi at TOP of CDC 2D tracks */
    TH1F* h_slot_CDC = nullptr;
    /** slot map of TOP */
    TH1F* h_slot_TOP = nullptr;

    /** ECL cluster energy */
    TH1F* h_E_ECL = nullptr;
    /** ECL cluster theta */
    TH1F* h_theta_ECL = nullptr;
    /** ECL cluster phi */
    TH1F* h_phi_ECL = nullptr;

    /** Jitter of CDC 2D -> L1 */
    TH1F* h_CDCL1 = nullptr;
    /** Jitter of ECL -> L1 */
    TH1F* h_ECLL1 = nullptr;
    /** Jitter of ECl 2nd input -> L1 */
    TH1F* h_ECLL1_2nd = nullptr;
    /** Jitter of TOP -> L1 */
    TH1F* h_TOPL1 = nullptr;
    /** Jitter of KLM -> L1 */
    TH1F* h_KLML1 = nullptr;
    /** Jitter of CDC 3D -> L1 */
    TH1F* h_CDC3DL1 = nullptr;
    /** Jitter of CDC NN -> L1 */
    TH1F* h_CDCNNL1 = nullptr;
    /** Jitter of Short track -> L1 */
    TH1F* h_TSFL1 = nullptr;
    /** Jitter of B2L window -> L1 */
    TH1F* h_B2LL1 = nullptr;

    /** Short track map */
    TH1F* h_map_ST = nullptr;
    /** Short track map */
    TH1F* h_map_ST2 = nullptr;
    /** Full track veto map */
    TH1F* h_map_veto = nullptr;
    /** TSF0 map */
    TH1F* h_map_TSF0 = nullptr;
    /** TSF2 map */
    TH1F* h_map_TSF2 = nullptr;
    /** TSF4 map */
    TH1F* h_map_TSF4 = nullptr;
    /** TSF1 map */
    TH1F* h_map_TSF1 = nullptr;
    /** TSF3 map */
    TH1F* h_map_TSF3 = nullptr;

    /** Wirecnt from TSF0 */
    TH1F* h_wc_TSF0 = nullptr;
    /** Wirecnt from TSF1 */
    TH1F* h_wc_TSF1 = nullptr;
    /** Wirecnt from TSF2 */
    TH1F* h_wc_TSF2 = nullptr;
    /** Wirecnt from TSF3 */
    TH1F* h_wc_TSF3 = nullptr;
    /** Wirecnt from TSF4 */
    TH1F* h_wc_TSF4 = nullptr;
    /** Wirecnt from TSF5 */
    TH1F* h_wc_TSF5 = nullptr;
    /** Wirecnt from TSF6 */
    TH1F* h_wc_TSF6 = nullptr;
    /** Wirecnt from all TSFs */
    TH1F* h_wc_sum = nullptr;
    /** Wirecnt from all TSFs from the injection BG clean region*/
    TH1F* h_wcsum_clean = nullptr;
    /** Wirecnt from all TSFs from the HER injection BG region*/
    TH1F* h_wcsum_injHER = nullptr;
    /** Wirecnt from all TSFs from the LER injection BG region*/
    TH1F* h_wcsum_injLER = nullptr;

    /** 2D plot: TSF0 cnt vs. time since injection (ms) */
    TH2F* h_wc0_injtime = nullptr;
    /** 2D plot: TSF1 cnt vs. time since injection (ms) */
    TH2F* h_wc1_injtime = nullptr;
    /** 2D plot: TSF2 cnt vs. time since injection (ms) */
    TH2F* h_wc2_injtime = nullptr;
    /** 2D plot: TSF3 cnt vs. time since injection (ms) */
    TH2F* h_wc3_injtime = nullptr;
    /** 2D plot: TSF4 cnt vs. time since injection (ms) */
    TH2F* h_wc4_injtime = nullptr;
    /** 2D plot: TSF5 cnt vs. time since injection (ms) */
    TH2F* h_wc5_injtime = nullptr;
    /** 2D plot: TSF6 cnt vs. time since injection (ms) */
    TH2F* h_wc6_injtime = nullptr;
    /** 2D plot: all TSFs cnt vs. time since injection (ms) */
    TH2F* h_wcsum_injtime = nullptr;

    //! Array to access the event time information from the trigger and FTSW
    StoreObjPtr<EventLevelTriggerTimeInfo> m_trgTime;

    //! DB pointerto access the hardware clock information
    DBObjPtr<HardwareClockSettings> m_hwclkdb;
  };

}

#endif
