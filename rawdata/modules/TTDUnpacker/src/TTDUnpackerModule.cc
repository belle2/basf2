/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <rawdata/modules/TTDUnpacker/TTDUnpackerModule.h>
#include <TDirectory.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TTDUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TTDUnpackerModule::TTDUnpackerModule() : Module(),  m_EventLevelTriggerTimeInfo()
{
  //Set module properties
  setDescription("TTD Unpacker");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("EventLevelTriggerTimeInfoName", m_EventLevelTriggerTimeInfoName, "The name of the StoreArray of TTD to be processed",
           std::string(""));
}

void TTDUnpackerModule::initialize()
{
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failing
  m_EventLevelTriggerTimeInfo.registerInDataStore(m_EventLevelTriggerTimeInfoName,
                                                  DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
}

void TTDUnpackerModule::event()
{
  m_EventLevelTriggerTimeInfo.create();// defaults to be invalid
  for (auto& it : m_rawTTD) {
    B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
            (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
            it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

    m_EventLevelTriggerTimeInfo.assign(new EventLevelTriggerTimeInfo(true, it.GetIsHER(0), (it.GetFrameCount(0) & 0x1) != 0,
                                       it.GetTimeSinceLastInjection(0), it.GetTimeSincePrevTrigger(0),
                                       it.GetBunchNumber(0)));
    break;
  }
}
