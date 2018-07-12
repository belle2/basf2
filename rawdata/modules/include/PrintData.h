//+
// File : PrintData.h
// Description : Read RawCOPPER objects and shows their hex dump
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef PRINTDATA_H
#define PRINTDATA_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>

//#include <daq/dataflow/EvtSocket.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>




#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawPXD.h>

//#include <daq/rawdata/modules/DAQConsts.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>



#include "TROOT.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1.h"


namespace Belle2 {

  /*! Read RawCOPPER objects and shows their hex dump */

  class PrintDataModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PrintDataModule();
    virtual ~PrintDataModule();

    //! initialization
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    //! print the contents of a RawCOPPER event
    virtual void printCOPPEREvent(RawCOPPER* raw_array, int i, int array_index);

    //! print the contents of a RawFTSW event
    virtual void printFTSWEvent(RawDataBlock* raw_array, int i);

    //! print the hex dump of an input
    virtual void printBuffer(int* buf, int nwords);

    //! print a PXD event
    virtual void printPXDEvent(RawPXD* raw_pxd);

    virtual void endRun();

  protected :
    //!Compression parameter
    int m_compressionLevel;

    //! No. of sent events
    int n_basf2evt;

    //! # of FTSWs
    int m_nftsw;

    //! # of COPPERs
    int m_ncpr;

    //! counter
    int m_print_cnt;

    //! histogram array
    TH1F* hist[10];

    //! Unix time of the run start
    unsigned int  m_start_utime;

    //! Time of the previous event
    timeval  prev_tv[1000];

    //! Integer time of the previous event
    unsigned int  prev_tv_eve[1000];

    //! previous timevalue
    int prev_tv_pos;

    //! flag of timevalue
    int tv_flag[1000];

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
