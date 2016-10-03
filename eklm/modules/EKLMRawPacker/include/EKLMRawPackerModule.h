/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMRAWPACKER_H
#define EKLMRAWPACKER_H

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
#include <map>
#include <iostream>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class EKLMRawPackerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    EKLMRawPackerModule();
    virtual ~EKLMRawPackerModule();

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

    //! Number of events
    int n_basf2evt;

    //! Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! RawKLM array
    StoreArray<RawKLM> rawklmarray;

    void formatData(int forward, int layer, int sector, int plane, int strip, int charge, float ctime, unsigned short& bword1,
                    unsigned short& bword2, unsigned short& bword3, unsigned short& bword4);

    //! to map logical coordinates to hardware coordinates
    std::map<int, int> m_ModuleIdToelectId;
    //! fill m_ModuleIdToelectId from xml file
    void loadMap();
  };

} // end namespace Belle2

#endif // MODULEHELLO_H
