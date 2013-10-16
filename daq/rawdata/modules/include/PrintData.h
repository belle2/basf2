//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/rawdata/modules/DAQConsts.h>

#include <daq/dataobjects/RawDataBlock.h>
#include <daq/dataobjects/RawFTSW.h>
#include <daq/dataobjects/RawCOPPER.h>
#include <daq/dataobjects/RawSVD.h>
#include <daq/dataobjects/RawCDC.h>
#include <daq/dataobjects/RawBPID.h>
#include <daq/dataobjects/RawEPID.h>
#include <daq/dataobjects/RawECL.h>
#include <daq/dataobjects/RawKLM.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>



namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class PrintDataModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PrintDataModule();
    virtual ~PrintDataModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();
    virtual void VerifyCheckSum(int* buf);
    virtual void PrintCOPPEREvent(RawCOPPER* raw_array, int i);
    virtual void PrintEvent(RawDataBlock* raw_array, int i);


    // Data members

  protected :

    // Event Meta Data
    //    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! data
    //    StoreObjPtr<RawCOPPER> m_rawcopper;


    //! size of buffer for one event (word)
    int BUF_SIZE_WORD;

    //!Compression parameter
    int m_compressionLevel;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int n_basf2evt;

    int* m_buffer;

    int m_nftsw;

    int m_ncdc;

    int m_fina_nwords;

    int m_finb_nwords;

    int m_finc_nwords;

    int m_find_nwords;

    int m_upper16_ftsw_utime;

    int m_upper16_cdc_utime;

    int m_prev_ftsw_time16;

    int m_prev_cdc_time16;

    double m_start_ftsw_time;

    double m_start_cdc_time;

    double m_last_utime_ftsw;

    double m_last_utime_cdc;

    int m_prev_ftsw_eve16;

    int m_prev_cdc_eve16;

    int m_cnt_ftsw_evejump;

    int m_cnt_cdc_evejump;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
