//+
// File : Raw2Ds.h
// Description : Module to restore RawData in DataStore through RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 26 - Sep - 2013
//-

#ifndef RAW2DSMODULE_H
#define RAW2DSMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawCOPPER.h>
//#include <rawdata/dataobjects/RawHeader.h>
//#include <rawdata/dataobjects/RawTrailer.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>


#include <framework/dataobjects/EventMetaData.h>

#define RBUFSIZE   100000000
#define MAXEVTSIZE  10000000

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Raw2DsModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Raw2DsModule();
    virtual ~Raw2DsModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    void setErrorFlag(unsigned int error_flag, StoreObjPtr<EventMetaData> evtmetadata);

  private:
    //! function to register data buffer in DataStore as RawCOPPERs
    void registerRawCOPPERs();

    // Data members
  private:

    //! RingBuffer ID
    std::string m_rbufname;
    RingBuffer* m_rbuf;

    //! No. of sent events
    int m_nevt;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
