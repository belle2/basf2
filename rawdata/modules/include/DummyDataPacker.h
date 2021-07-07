/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DUMMYDATAPACKER_H
#define DUMMYDATAPACKER_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>


namespace Belle2 {

  /*! Module to store dummy data in RawCOPPER object */

  class DummyDataPackerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DummyDataPackerModule();
    virtual ~DummyDataPackerModule();

    //! initialization
    virtual void initialize() override;

    //! event module
    virtual void event() override;


  protected :
    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //    StoreArray<RawFTSW> raw_ftswarray;

    //! # of events in a run
    int max_nevt;

    //! Node ID
    //    int m_nodeid;

    //! event counter
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

    //! RawTOP array
    StoreArray<RawTOP> raw_bpidarray;

    //! RawARICH array
    StoreArray<RawARICH> raw_epidarray;

    //! RawECL array
    StoreArray<RawECL> raw_eclarray;

    //! RawKLM array
    StoreArray<RawKLM> raw_klmarray;

    //! RawARICH array
    StoreArray<RawTRG> raw_trgarray;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
