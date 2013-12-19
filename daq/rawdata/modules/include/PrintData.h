//+
// File : PrintData.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTDATA_H
#define PRINTDATA_H

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

#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawBPID.h>
#include <rawdata/dataobjects/RawEPID.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>


#include <framework/core/HistoModule.h>
#include "TH1F.h"
#include "TH2F.h"

#define SIZE_CDC_1_NUM 1000
#define SIZE_CDC_1_MIN 0.
#define SIZE_CDC_1_MAX 5000.
#define SIZE_CDC_2_NUM 1000
#define SIZE_CDC_2_MIN 0.
#define SIZE_CDC_2_MAX 5000.
#define SIZE_UT3_NUM 1000
#define SIZE_UT3_MIN 0.
#define SIZE_UT3_MAX 5000.

#define TDIFF_EVE_NUM 1000
#define TDIFF_EVE_MIN 0.
#define TDIFF_EVE_MAX 0.01

#define EVEDIFF_NUM 1000
#define EVEDIFF_MIN -500.
#define EVEDIFF_MAX 500.

#define TRGRATE_NUM 3000
#define TRGRATE_MIN 0.
#define TRGRATE_MAX 3000.

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */



  /*   class Histo1D : public TObject { */
  /*   public : */
  /*     Histo1D(){} */
  /*     ~Histo1D(); */
  /*     virtual void Fill( double value ); */
  /*     virtual void Set( int, double, double); */
  /*     virtual void Print(); */
  /*   private : */
  /*     double min; */
  /*     double max; */
  /*     int array_size; */
  /*     int* array; */
  /*     ClassDef( Histo1D, 1); */
  /*   }; */

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
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i);
    virtual void printFTSWEvent(RawDataBlock* raw_array, int i);
    virtual void printBuffer(int* buf, int nwords);
    virtual void defineHisto();
    virtual void fillHisto1D(double, int*, int, double, double);
    virtual void fillHisto2D(double, double, int*, int, double, double,  int, double, double);
    virtual void printArray(int* array, int array_size, double min, double max, const char*);
    virtual void endRun();
    virtual void terminate();

  protected :
    //!Compression parameter
    int m_compressionLevel;
    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int n_basf2evt;

    int m_nftsw;

    int m_ncpr;

    TH1* h_size;
    TH1* h_nhit;
    TH2* h_winfadc[48];
    TH2* h_wintdc[48];
    TH2* h_chfadc;
    TH2* h_chtdc;


    int  m_size_cdc_1[SIZE_CDC_1_NUM];
    int  m_size_cdc_2[SIZE_CDC_2_NUM];
    int  m_size_ut3[SIZE_UT3_NUM];
    int  m_tdiff_eve[TDIFF_EVE_NUM] ;

    int m_tdiff_cdc_1[ TDIFF_EVE_NUM ];
    int m_tdiff_cdc_2[ TDIFF_EVE_NUM ];
    int m_tdiff_ut3[ TDIFF_EVE_NUM ];

    int m_eve_diff[ EVEDIFF_NUM ];
    int m_eve_diff_prev[ EVEDIFF_NUM ];

    int m_rate[ TRGRATE_NUM ];

    double m_prev_time_cdc_1;
    double m_prev_time_cdc_2;
    double m_prev_time_ut3;

    double m_time_cdc_1;
    double m_time_cdc_2;
    double m_time_ut3;

    unsigned int m_eve_cnt;
    unsigned int m_eve_from_data;
    unsigned int m_prev_eve_from_data;

    unsigned int m_eve_from_ftsw;

    double m_start_time;

  };


} // end namespace Belle2

#endif // MODULEHELLO_H
