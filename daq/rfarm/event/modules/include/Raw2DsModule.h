/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAW2DSMODULE_H
#define RAW2DSMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>

#include <string>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawCOPPER.h>

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
    void initialize() override;

    //! Module functions to be called from event process
    void beginRun() override;
    void event() override;
    void endRun() override;
    void terminate() override;

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

    StoreObjPtr<EventMetaData> m_eventMetaData; /**< EventMetaData. */
    StoreArray<RawDataBlock> m_rawDataBlock; /**< RawDataBlock. */
    StoreArray<RawCOPPER> m_rawCOPPER; /**< RawCOPPER. */
    StoreArray<RawCDC> m_rawCDC; /**< RawCDC. */
    StoreArray<RawSVD> m_rawSVD; /**< RawSVD. */
    StoreArray<RawTOP> m_rawTOP; /**< RawTOP. */
    StoreArray<RawECL> m_rawECL; /**< RawECL. */
    StoreArray<RawARICH> m_rawARICH; /**< RawARICH. */
    StoreArray<RawKLM> m_rawKLM; /**< RawKLM. */
    StoreArray<RawTRG> m_rawTRG; /**< RawTRG. */
    StoreArray<RawFTSW> m_rawFTSW; /**< RawFTSW. */
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
