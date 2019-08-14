//+
// File : BKLMRawPackerModule.h
// Description : Module to store dummy data in RawKLM object
//
// Author : Anselm Vossen, based on Satoru Yamada's DummyDataPacker
//-

#pragma once

#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <framework/database/DBObjPtr.h>
#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <bklm/dbobjects/BKLMElectronicsMap.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <iostream>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class BKLMRawPackerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    BKLMRawPackerModule();
    virtual ~BKLMRawPackerModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! begin run functions to be called from main process
    virtual void beginRun() override;

    //! end run functions to be called from main process
    virtual void endRun() override;

    //! Module functions to be called from event process
    virtual void event() override;


  private:
    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //    StoreArray<RawFTSW> raw_ftswarray;

    //! offset of the scintillator ADC
    int m_scintADCOffset = 3400;

    //! # of events in a run
    int max_nevt;

    //! Node ID
    int m_nodeid;

    //! Number of events
    int n_basf2evt;

    //! Messaage handler
    //    MsgHandler* m_msghandler;

    /*     //! Compression Level */
    /*     int m_compressionLevel; */

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawKLM array
    StoreArray<RawKLM> rawklmarray;

    //! digits array
    StoreArray<BKLMDigit> digits;

    //! ADC offset and threshold read from database
    DBObjPtr<BKLMADCThreshold> m_ADCParams;

    //! Electronics map.
    DBObjPtr<BKLMElectronicsMap> m_ElectronicsMap;

    //! format the data
    //! @param[in] channel rpc channel
    //! @param[in] axis phi or z
    //! @param[in] lane slot in the crate for the rpc
    //! @param[in] tdc measured tdc value
    //! @param[in] charge num photons for the scintillators
    //! @param[in] ctime the ctime (or is this already in the copper header)
    //! @param[out] bword1 first 16bit word
    //! @param[out] bword2 second 16bit word
    //! @param[out] bword3 third 16bit word
    //! @param[out] bword4 forth 16bit word
    void formatData(int flag, int channel, int axis, int lane, int tdc, int charge, int ctime, unsigned short& bword1,
                    unsigned short& bword2,
                    unsigned short& bword3, unsigned short& bword4);

  };

} // end namespace Belle2
