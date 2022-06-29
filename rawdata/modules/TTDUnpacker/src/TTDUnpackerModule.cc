/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <rawdata/modules/TTDUnpacker/TTDUnpackerModule.h>
#include <TDirectory.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TTDUnpacker);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TTDUnpackerModule::TTDUnpackerModule() : Module(),  m_EventLevelTriggerTimeInfo()
{
  //Set module properties
  setDescription("TTD Unpacker, unpacks first RawFTSW object");
  setPropertyFlags(c_ParallelProcessingCertified);

}

void TTDUnpackerModule::initialize()
{
  m_rawTTD.isOptional(); /// TODO better use isRequired(), but RawFTSW is not in sim, thus tests are failing
  m_EventLevelTriggerTimeInfo.registerInDataStore(DataStore::EStoreFlags::c_ErrorIfAlreadyRegistered);
}

void TTDUnpackerModule::event()
{
  m_EventLevelTriggerTimeInfo.create();// defaults to be invalid
  for (auto& it : m_rawTTD) {
    it.SetVersion();
    if (it.m_version >= 2) {
      // attention, accessing a non supported variable results in B2FATAL :-/
      B2DEBUG(29, "TTD FTSW : " << hex << it.GetTTUtime(0) << " " << it.GetTTCtime(0) << " EvtNr " << it.GetEveNo(0)  << " Type " <<
              (it.GetTTCtimeTRGType(0) & 0xF) << " TimeSincePrev " << it.GetTimeSincePrevTrigger(0) << " TimeSinceInj " <<
              it.GetTimeSinceLastInjection(0) << " IsHER " << it.GetIsHER(0) << " Bunch " << it.GetBunchNumber(0));

      m_EventLevelTriggerTimeInfo->setIsHER(it.GetIsHER(0));
      m_EventLevelTriggerTimeInfo->setRevo2((it.GetFrameCount(0) & 0x1) != 0);
      m_EventLevelTriggerTimeInfo->setTimeSinceLastInjection(it.GetTimeSinceLastInjection(0));
      m_EventLevelTriggerTimeInfo->setTimeSincePrevTrigger(it.GetTimeSincePrevTrigger(0));
      m_EventLevelTriggerTimeInfo->setBunchNumber(it.GetBunchNumber(0));
      m_EventLevelTriggerTimeInfo->setValid();
    }
    break;
  }
}
