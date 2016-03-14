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

#define USE_RAWDATABLOCK

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DummyDataSourceModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DummyDataSourceModule();
    virtual ~DummyDataSourceModule();

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
    int m_nodeid;

    //! Number of events
    int n_basf2evt;

    //! Messaage handler
    MsgHandler* m_msghandler;

    /*     //! Compression Level */
    /*     int m_compressionLevel; */

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawCOPPER array
    StoreArray<RawCOPPER> rawcprarray;

#ifdef USE_RAWDATABLOCK
    //! RawDataBlock array
    StoreArray<RawDataBlock> raw_datablkarray;
#endif

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
