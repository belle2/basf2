//+
// File : DAQPerf.h
// Description : Module to measure data transfer performance
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Jan - 2012
//-

#ifndef DAQPERF_H
#define DAQPERF_H

#include <framework/core/Module.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawCOPPER.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DAQPerfModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DAQPerfModule();
    virtual ~DAQPerfModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:
    timeval m_t0;
    int m_nevent;
    int m_ncycle;
    bool m_mon;
    double m_totbytes;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
