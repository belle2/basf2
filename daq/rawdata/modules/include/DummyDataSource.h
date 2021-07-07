/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DUMMYDATAPACKER_H
#define DUMMYDATAPACKER_H

#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/core/Module.h>
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
    unsigned int n_basf2evt;

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

    double m_totbytes;
    //! Node name
    std::string m_nodename;

    //! Use shared memory
    int m_shmflag;

    //! start flag
    int m_start_flag;

    double m_start_time;
    double m_prev_time;
    int prev_event;
    double m_prev_totbytes;
    int m_prev_nevt;

    //! store time info.
    double getTimeSec()
    {
      struct timeval t;
      gettimeofday(&t, NULL);
      return (t.tv_sec + t.tv_usec * 1.e-6);
    }


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
