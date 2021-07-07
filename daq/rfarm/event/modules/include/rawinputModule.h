/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef RAW_INPUT_H
#define RAW_INPUT_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <string>

#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawCOPPER.h>

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

#endif
