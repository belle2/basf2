//+
// File : BKLMRawPackerModule.h
// Description : Module to store dummy data in RawKLM object
//
// Author : Anselm Vossen, based on Satoru Yamada's DummyDataPacker
//-

#ifndef BKLMRAWPACKER_H
#define BKLMRAWPACKER_H

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

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawKLM.h>
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
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  private:
    //! check data contents
    //    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //    StoreArray<RawFTSW> raw_ftswarray;

    //! use default elect id, if not found in mapping file
    bool m_useDefaultElectId = true;

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

    //! to map logical coordinates to hardware coordinates
    std::map<int, int> m_ModuleIdToelectId;

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
    void formatData(int channel, int axis, int lane, int tdc, int charge, int ctime, unsigned short& bword1, unsigned short& bword2,
                    unsigned short& bword3, unsigned short& bword4);

    //    //! put datapackets for a track together
    //    //! param[in] channel The channel that will be set in all layers
    //     //! param[out] bword1-bword4 the words
    //    void getTrack(int channel, short& bword1, short& bword2, short& bword3, short& bword4);

    //! fill m_ModuleIdToelectId from xml file
    void loadMap();

    //! To be used to map module id to electronics address
    //! @param copperId id of the copper board
    //! @param finesse is the Finesse slot on the copper boards
    void intToElectCoo(int id, int& copper, int& finesse, int& lane, int& plane);

    //! TO be used to map electronics address to module id
    int electCooToInt(int copper, int finesse, int lane, int axis);


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
