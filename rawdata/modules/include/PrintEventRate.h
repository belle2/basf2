//+
// File : PrintEventRate.h
// Description : Print event rate of input data
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTEVENTRATE_H
#define PRINTEVENTRATE_H
#include <stdlib.h>
#include <string>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>


#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/dataobjects/RawTRG.h>


namespace Belle2 {

  /*! Print event rate of input data */

  class PrintEventRateModule : public Module {

    // Public functions
  public:

    //! Constructor
    PrintEventRateModule();

    //! Destructor
    virtual ~PrintEventRateModule();

    //! Module function to be called from event process
    virtual void event();

    //! Module function to be called from main process
    virtual void initialize();

    //! Module function to be called after main process
    virtual void endRun();

    //! print an event
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);

  protected:

    //! run number
    int m_run;

    //! subrun number
    int m_subrun;

    //! current unixtime
    unsigned int m_cur_utime;

    //! current event number
    unsigned int m_cur_event;

    //! unixtime at the beginning
    unsigned int m_start_utime;

    //! the first event number
    unsigned int m_start_event;

    //! previous unixtime
    unsigned int m_prev_utime;

    //! previous event number
    unsigned int m_prev_event;

    //! event size
    int m_size;

    //! # of all COPPERs
    int m_cpr;

    //! # of COPPERs for SVD
    int m_cpr_svd;

    //! # of COPPERs for CDC
    int m_cpr_cdc;

    //! # of COPPERs for TOP
    int m_cpr_top;

    //! # of COPPERs for ARICH
    int m_cpr_arich;

    //! # of COPPERs for ECL
    int m_cpr_ecl;

    //! # of COPPERs for KLM
    int m_cpr_klm;

    //! # of COPPERs for TRG
    int m_cpr_trg;

    //! # of COPPERs for others
    int m_cpr_others;

    //! # of basf2 event counter
    unsigned int m_n_basf2evt;

    //! total processed data size
    double m_tot_bytes;

    //! previous value of processed data size
    double m_prev_tot_bytes;

    //! blocksize of RawDatablock object
    int m_datablk_blksize;

    //! blocksize of RawCOPPER object
    int m_cpr_blksize;

    //! blocksize of RawTLU object
    int m_tlu_blksize;

    //! blocksize of RawFTSW object
    int m_ftsw_blksize;

    //! blocksize of RawPXD object
    int m_pxd_blksize;

    //! blocksize of RawSVD object
    int m_svd_blksize;

    //! blocksize of RawCDC object
    int m_cdc_blksize;

    //! blocksize of RawTOP object
    int m_top_blksize;

    //! blocksize of RawARICH object
    int m_arich_blksize;

    //! blocksize of RawECl object
    int m_ecl_blksize;

    //! blocksize of RawKLM object
    int m_klm_blksize;

    //! blocksize of RawTRG object
    int m_trg_blksize;

    //! # of RawDatablock events
    int m_datablk_evecnt;

    //! # of RawSVD events
    int m_cpr_evecnt;

    //! # of RawTLU events
    int m_tlu_evecnt;

    //! # of RawFTSW events
    int m_ftsw_evecnt;

    //! # of RawDatablock events
    int m_pxd_evecnt;

    //! # of RawDatablock events
    int m_svd_evecnt;

    //! # of RawDatablock events
    int m_cdc_evecnt;

    //! # of RawDatablock events
    int m_top_evecnt;

    //! # of RawDatablock events
    int m_arich_evecnt;

    //! # of RawDatablock events
    int m_ecl_evecnt;

    //! # of RawDatablock events
    int m_klm_evecnt;

    //! # of RawDatablock events
    int m_trg_evecnt;

    //! # of RawDatablock events
    int m_evecnt;

    //! output interval
    int m_print_interval;

    //! first event number
    int m_first_evt;

    //! # of error events
    int m_erreve;

    //! # of error RawCOPPER blocks
    int m_errcpr;

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
