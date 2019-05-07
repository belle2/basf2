#ifndef TRCGRLDQMMODULE_h
#define TRCGRLDQMMODULE_h

#include "trg/grl/dataobjects/TRGGRLUnpackerStore.h"
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <stdlib.h>
#include <iostream>
#include <string>

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
    TDirectory* oldDir;
    TDirectory* dirDQM;

    /** Number of tracks */
    TH1I* h_N_track;

    /** phi_i of CDC 2D tracks */
    TH1F* h_phi_i;
    /** phi_CDC, extrapolated phi at ECL of CDC 2D tracks */
    TH1F* h_phi_CDC;
    /** sector_CDC, extrapolated phi at KLM of CDC 2D tracks */
    TH1F* h_sector_CDC;
    /** sector map of KLM */
    TH1F* h_sector_KLM;
    /** slot_CDC, extrapolated phi at TOP of CDC 2D tracks */
    TH1F* h_slot_CDC;
    /** slot map of TOP */
    TH1F* h_slot_TOP;

    /** ECL cluster energy */
    TH1F* h_E_ECL;
    /** ECL cluster theta */
    TH1F* h_theta_ECL;
    /** ECL cluster phi */
    TH1F* h_phi_ECL;

    /** Jitter of CDC 2D -> L1 */
    TH1F* h_CDCL1;
    /** Jitter of ECL -> L1 */
    TH1F* h_ECLL1;
    /** Jitter of ECl 2nd input -> L1 */
    TH1F* h_ECLL1_2nd;
    /** Jitter of TOP -> L1 */
    TH1F* h_TOPL1;
    /** Jitter of KLM -> L1 */
    TH1F* h_KLML1;
    /** Jitter of CDC 3D -> L1 */
    TH1F* h_CDC3DL1;
    /** Jitter of CDC NN -> L1 */
    TH1F* h_CDCNNL1;
    /** Jitter of Short track -> L1 */
    TH1F* h_TSFL1;
    /** Jitter of B2L window -> L1 */
    TH1F* h_B2LL1;

    /** Short track map */
    TH1F* h_map_ST;
    /** Full track veto map */
    TH1F* h_map_veto;
    /** TSF0 map */
    TH1F* h_map_TSF0;
    /** TSF2 map */
    TH1F* h_map_TSF2;
    /** TSF4 map */
    TH1F* h_map_TSF4;
  };

}

#endif
