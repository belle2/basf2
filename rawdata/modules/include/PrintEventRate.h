//+
// File : PrintEventRate.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
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

  /*! A class definition of an input module for Sequential ROOT I/O */

  class PrintEventRateModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PrintEventRateModule();
    virtual ~PrintEventRateModule();

    //! Module functions to be called from event process
    virtual void event();
    virtual void initialize();
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);
    virtual void endRun();

  protected:
    int m_run;
    int m_subrun;
    unsigned int m_cur_utime;
    unsigned int m_cur_event;
    unsigned int m_start_utime;
    unsigned int m_start_event;
    unsigned int m_prev_utime;
    unsigned int m_prev_event;
    int m_size;

    int m_cpr;
    int m_cpr_svd;
    int m_cpr_cdc;
    int m_cpr_top;
    int m_cpr_arich;
    int m_cpr_ecl;
    int m_cpr_klm;
    int m_cpr_trg;
    int m_cpr_others;

    unsigned int m_n_basf2evt;
    double m_tot_bytes;
    double m_prev_tot_bytes;

    // For distribution over COPPERs

    int m_datablk_blksize;
    int m_cpr_blksize;
    int m_tlu_blksize;
    int m_ftsw_blksize;
    int m_pxd_blksize;
    int m_svd_blksize;
    int m_cdc_blksize;
    int m_top_blksize;
    int m_arich_blksize;
    int m_ecl_blksize;
    int m_klm_blksize;
    int m_trg_blksize;

    int m_datablk_evecnt;
    int m_cpr_evecnt;
    int m_tlu_evecnt;
    int m_ftsw_evecnt;
    int m_pxd_evecnt;
    int m_svd_evecnt;
    int m_cdc_evecnt;
    int m_top_evecnt;
    int m_arich_evecnt;
    int m_ecl_evecnt;
    int m_klm_evecnt;
    int m_trg_evecnt;
    int m_evecnt;

    int m_print_interval;

    int m_first_evt;

    int m_erreve;
    int m_errcpr;

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
