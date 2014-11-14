//+
// File : Convert2RawDet.h
// Description : Module to convert from RawCOPPER or RawDataBlock to RawDetector objects
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 24 - Oct - 2014
//-

#ifndef CONVERT2RAWDET_H
#define CONVERT2RAWDET_H

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

//#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawEPID.h>
#include <rawdata/dataobjects/RawBPID.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawCOPPER.h>

#include <framework/dataobjects/EventMetaData.h>


namespace Belle2 {

  /*! A class definition of a module to convert from RawCOPPER or RawDataBlock to RawDetector objects */

  class Convert2RawDetModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Convert2RawDetModule();
    virtual ~Convert2RawDetModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:

    //! function to register data buffer in DataStore as RawDetector
    void convertDataObject(RawDataBlock* raw_dblk);

    // Data members
  private:

    //! No. of sent events
    int m_nevt;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
