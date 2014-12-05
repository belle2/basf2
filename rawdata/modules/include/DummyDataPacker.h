//+
// File : DummyDataPacker.h
// Description : Module to store dummy data in RawCOPPER object
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 14 - Jul - 2014
//-

#ifndef DUMMYDATAPACKER_H
#define DUMMYDATAPACKER_H

#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawBPID.h>
#include <rawdata/dataobjects/RawEPID.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DummyDataPackerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DummyDataPackerModule();
    virtual ~DummyDataPackerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  protected :
    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //    StoreArray<RawFTSW> raw_ftswarray;

    //! # of events in a run
    int max_nevt;

    //! Node ID
    //    int m_nodeid;

    //! Number of events
    int n_basf2evt;

    //! Messaage handler
    //    MsgHandler* m_msghandler;

    /*     //! Compression Level */
    /*     int m_compressionLevel; */

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawCOPPER array
    StoreArray<RawCOPPER> raw_cprarray;

    //! RawSVD array
    StoreArray<RawSVD> raw_svdarray;

    //! RawCDC array
    StoreArray<RawCDC> raw_cdcarray;

    //! RawBPID array
    StoreArray<RawBPID> raw_bpidarray;

    //! RawEPID array
    StoreArray<RawEPID> raw_epidarray;

    //! RawECL array
    StoreArray<RawECL> raw_eclarray;

    //! RawKLM array
    StoreArray<RawKLM> raw_klmarray;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
