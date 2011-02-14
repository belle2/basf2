//+
// File : genrawadata.h
// Description : Pseudo raw data generator for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef GEN_RAWDATAMODULE_H
#define GEN_RAWDATAODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <daq/rawdata/RawCDC.h>
#include <daq/rawdata/RawHeader.h>

namespace Belle2 {

  class GenRawDataModule : public Module {

  public:

    //! Constructor
    GenRawDataModule();

    //! Destructor
    virtual ~GenRawDataModule();

    //! Initialize the Module
    virtual void initialize();

    //! Called when entering a new run
    virtual void beginRun();

    //! Running over all events
    virtual void event();

    //! Is called after processing the last event of a run
    virtual void endRun();

    //! Is called at the end of your Module
    virtual void terminate();

  private:
    unsigned int* m_buffer;
    int m_size;
    int m_maxevt;
    int m_nevt;

  };
}

#endif /* EXERCISE1_H_ */
