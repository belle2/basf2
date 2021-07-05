/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGGRLDQMModule.h
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@post.kek.jp
//---------------------------------------------------------------
// Description : DQM module for TRGGRL
//---------------------------------------------------------------
#ifndef TRCGRLDQMMODULE_h
#define TRCGRLDQMMODULE_h

#include <framework/core/HistoModule.h>

#include <TH1I.h>
#include <TH1F.h>

namespace Belle2 {

  class TRGGRLDQMModule : public HistoModule {

  public:
    //! Costructor
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
  };

}

#endif
