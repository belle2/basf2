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
#include <framework/logging/Logger.h>
#include <framework/database/DBObjPtr.h>
#include <bklm/dataobjects/BKLMDigit.h>
#include <bklm/dbobjects/BKLMADCThreshold.h>
#include <rawdata/dataobjects/RawKLM.h>

#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <map>
#include <string>
#include <iostream>



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

    //! fill m_electIdToModuleId from Data Base
    void loadMapFromDB();

    //!get the module id from the electroncis coordinates
    //! @param copperId id of the copper board
    //! @param finesseNum The Finesse slot on the copper boards
    //! @param lane the lane number, giving for the rpcs the slot number in the crate
    //! @param channel the channel
    //! @param axis z or phi
    // int getModuleId(int copperId, int finesseNum, int lane, int channel, int axis);

    //!in case the module id is not found in the mapping and useDefaultModuleId flag is set, this computes the default
    //!module id from the lane and the axis. Sector etc are set to 0
    //! @param lane the lane number, giving for the rpcs the slot number in the crate
    //! @param axis z or phi
    int getDefaultModuleId(int copperId, int finesse, int lane, int axis, int channel, bool& outOfRange);

    //! To be used to map electronics address to module id
    //! @param copperId id of the copper board
    //! @param finesseNum The Finesse slot on the copper boards
    //! @param lane the lane number, giving for the rpcs the slot number in the crate
    //! @param axis the axis bit in the datapacket
    int electCooToInt(int copper, int finesse, int lane, int axis, int channel);

    //! remap the channel ID for scitilators and RPCs
    unsigned short getChannel(int isForward, int sector, int layer,  int plane,  unsigned short channel);

    //! handle 0-->max max-->0 channel number flip between software and detector
    unsigned short flipChannel(int isForward, int sector, int layer, int plane, unsigned short channel, bool& isOutRange);

    //    void intToElectCoo(int id, int& copper, int& finesse, int& lane);
    //! to map hardware coordinates to logical coordinates
    std::map<int, int> m_electIdToModuleId;

    //! length of one hit in 4 byte words. This is needed find the hits in the detector buffer
    const int hitLength = 2;

    //! the flag to keep the Even packages
    bool m_keepEvenPackages = false;
    //!use default module id, if not found in mapping file
    bool m_useDefaultModuleId = false;

    //!use electronic map from DataBase or not
    bool m_loadMapFromDB = true;

    //! load threshold from DataBase (true) or not (false)
    bool m_loadThresholdFromDB = true;

    //! offset of the scintillator ADC
    int m_scintADCOffset = 3400;

    //! threshold for the scintillator NPE
    double m_scintThreshold = 140;

    //! is this real data (true) or MC data (false)
    bool m_rawdata;

    //! name of BKLMDigit store array
    std::string m_outputDigitsName;

    //! counter for channels that was rejected due to un-reasonable channel number
    long m_rejectedCount = 0;

    //! warnig message: number of channels rejected due to un-reasonable channel number
    std::map<std::string, long> m_rejected;

    //! ADC offset and threshold read from database
    DBObjPtr<BKLMADCThreshold> m_ADCParams;

    //! rawKLM StoreArray
    StoreArray<RawKLM> rawKLM;

    //! BKLMDigit Array
    StoreArray<BKLMDigit>bklmDigits;

  };



}
#endif
