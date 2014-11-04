/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anselm Vossen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef BLKMUnpackerModule_H
#define BLKMUnpackerModule_H

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>

#include <bklm/dataobjects/BKLMDigit.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <map>
#include <string>

using namespace std;

namespace Belle2 {

  //!Unpacker for the KLM
  class BKLMUnpackerModule : public Module {
  public:
    //! Constructor
    BKLMUnpackerModule();
    //! Destructor
    virtual ~BKLMUnpackerModule();
    //! Initialize at start of job
    virtual void initialize();
    //! begin run stuff
    virtual void beginRun();
    //! Unpack one event and create digits
    virtual void event();
    //! end run stuff
    virtual void endRun();
    //! Terminate at the end of job
    virtual void terminate();

  private:

    //! fill m_electIdToModuleId from xml file
    void loadMap();

    //!get the module id from the electroncis coordinates
    //! @param copperId id of the copper board
    //! @param finesseNum The Finesse slot on the copper boards
    //! @param lane the lane number, giving for the rpcs the slot number in the crate
    //! @param channel the channel
    //! @param axis z or phi
    int getModuleId(int copperId, int finesseNum, int lane, int channel, int axis);

    //! To be used to map electronics address to module id
    //! @param copperId id of the copper board
    //! @param finesseNum The Finesse slot on the copper boards
    //! @param lane the lane number, giving for the rpcs the slot number in the crate
    //! @param axis the axis bit in the datapacket
    int electCooToInt(int copper, int finesse, int lane, int axis);

    //    void intToElectCoo(int id, int& copper, int& finesse, int& lane);
    //! to map hardware coordinates to logical coordinates
    map<int, int> m_electIdToModuleId;

    //! length of one hit in 4 byte words. This is needed find the hits in the detector buffer
    const int hitLength = 2;
  };



}
#endif
