//+
// File : rawinput.h
// Description : Sequential ROOT I/O input module
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 2 - Dec - 2013
//-

#ifndef RAW_INPUT_H
#define RAW_INPUT_H

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


#include <string>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>


#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawEPID.h>
#include <rawdata/dataobjects/RawBPID.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawCOPPER.h>
//#include <rawdata/dataobjects/RawHeader.h>
//#include <rawdata/dataobjects/RawTrailer.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <framework/dataobjects/EventMetaData.h>

#define RBUFSIZE   100000000
#define MAXEVTSIZE  10000000

namespace Belle2 {
  /*! A class definition of an input module for Sequential ROOT I/O. */

  class RawInputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    RawInputModule();
    virtual ~RawInputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    //! function to read raw data
    void registerRawCOPPERs();

    // Data members
  private:
    //! File name
    std::string m_inputFileName;

    //! File handle
    int m_fd;

    //! Total nr. of events in the file
    int m_nevt;

  };
} // end namespace Belle2

#endif
