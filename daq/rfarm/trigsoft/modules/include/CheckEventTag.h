//+
// File : CheckEventTag.h
// Description : Module to check consistency of event tags among detectors
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 10 - Jan - 2014
//-

#ifndef CHECKEVENTTAGMODULE_H
#define CHECKEVENTTAGMODULE_H

#include <string>
#include <vector>

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>


#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawCOPPER.h>
//#include <rawdata/dataobjects/RawHeader.h>
//#include <rawdata/dataobjects/RawTrailer.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <framework/dataobjects/EventMetaData.h>

#define MAXCPR 20

namespace Belle2 {

  /*! A class definition of CheckEventTag module */

  class CheckEventTagModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    CheckEventTagModule();
    virtual ~CheckEventTagModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! No. of HLT events
    int m_nevt;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
