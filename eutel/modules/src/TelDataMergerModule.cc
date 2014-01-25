#include <eutel/modules/TelDataMergerModule.h>

// include standard c++
#include <memory>
#include <set>

// Load the eudaq part
#include <eutel/eudaq/DetectorEvent.h>
#include <eutel/eudaq/RawDataEvent.h>
#include <eutel/eudaq/PluginManager.h>
#include <eutel/eudaq/Utils.h>
#include <eutel/eudaq/TBTelEvent.h>

// load the datastore objects
#include <testbeam/vxd/dataobjects/TelDigit.h>
#include <testbeam/vxd/dataobjects/TelEventInfo.h>

#include <testbeam/vxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

// avoid having to wrap everything in the namespace explicitly
// only permissible in .cc files!
using namespace Belle2;

// this line registers the module with the framework and actually makes it available
// in steering files or the the module list (basf2 -m).
// Note that the 'Module' part of the class name is missing, this is also the way it
// will be called in the module list.
REG_MODULE(TelDataMerger);

// file-local variables
namespace {
  // static const unsigned TLUID = eudaq::Event::str2id("_TLU");
  static const unsigned IDMASK = 0x7fff;
}

TelDataMergerModule::TelDataMergerModule() : Module(),
  m_telEventNo(0), m_reader(NULL), m_nDataEvents(0), m_nBOREvents(0), m_nEOREvents(0),
  m_nNoTrigEvents(0), m_currentTLUTagFromFTSW(0), m_currentTLUTagFromEUDAQ(0)
{

  // Module Description
  setDescription("Data Merger Module for EUDET telescope data.");

  //Parameter definition
  addParam("inputFileName", m_inputFileName, "Input file name. For multiple files, use inputFileNames instead. Can be overridden using the -i argument to basf2.", std::string(""));
  addParam("storeDigitsName", m_storeDigitsName, "DataStore name of TelDigits collection", std::string(""));
}


TelDataMergerModule::~TelDataMergerModule()
{
}

void TelDataMergerModule::stopPeacefully()
{
  StoreObjPtr<EventMetaData> storeEventMetaData;
  storeEventMetaData->setEndOfData();
}

bool TelDataMergerModule::processBOREvent(const eudaq::Event& ev)
{
  // Process BORE event
  if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {
    // Initialize plugin manager
    eudaq::PluginManager::Initialize(* detEv);
  }
  ++m_nBOREvents;
  if (m_nBOREvents > 1) {
    B2WARNING("Multiple BOREs (" << m_nBOREvents << ")");
  }
  return true;
}

bool TelDataMergerModule::processEOREvent(const eudaq::Event&)
{
  ++m_nEOREvents;
  if (m_nEOREvents > 1) {
    B2WARNING("Multiple EOREs (" << m_nEOREvents << ")");
  }
  return true;
}

int TelDataMergerModule::getTelTriggerID(const eudaq::Event& ev)
{

  if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

    bool bFoundTrigId = false;

    for (size_t i = 0; i < (*detEv).NumEvents(); ++i) {
      const eudaq::Event* subEv = (*detEv).GetEvent(i);
      // the first time we find a SubEvent which is of type EUDRB or NI,
      // we collect its Trigger ID and assume it is the trigger ID
      // of the whole event
      if (std::string("EUDRB") == subEv->GetSubType() ||
          std::string("NI") == subEv->GetSubType()) {
        m_currentTLUTagFromEUDAQ =
          static_cast<int>((eudaq::PluginManager::GetTriggerID(* subEv) & IDMASK));
        bFoundTrigId = true;
        break;
      }
    }

    for (size_t i = 0; i < (*detEv).NumEvents(); ++i) {
      const eudaq::Event* subEv = (*detEv).GetEvent(i);
      B2DEBUG(15, "  TrigID  " << (eudaq::PluginManager::GetTriggerID(* subEv) & IDMASK)
              << "  (" << subEv->GetSubType() << ")");
    }
    return m_currentTLUTagFromEUDAQ;

    if (! bFoundTrigId) {
      ++m_nNoTrigEvents;
      B2WARNING("No event of \"EUDRB\" subtype found! \n Could not extract Trigger ID. \n Will skip this event.");
      return -1;
    }

  } else return -1;

}

bool TelDataMergerModule::processNormalEvent(const eudaq::Event& ev)
{
  // Setup Belle2 Datastore
  StoreArray<TelDigit> storeTelDigits(m_storeDigitsName);
  StoreObjPtr<TelEventInfo> storeTelEventInfo;

  if (m_nBOREvents == 0) {
    B2WARNING("No BORE found prior to data event!");
  }

  ++m_nDataEvents;

  // This is just a cast, no conversion, so never mind that it is duplicated. Or?
  if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

    // it is really important to note here that we must manually set the trigger ID
    // after the conversion step has finished. This is due to the fact that a certain
    // TLU ID Mask was used, which in turn is not considered in the eudaq part.
    TBTelEvent tbEvt = eudaq::PluginManager::ConvertToTBTelEvent(* detEv);
    tbEvt.setTriggerId(m_currentTLUTagFromEUDAQ);

    B2DEBUG(10, "TBEvent: Event: " << tbEvt.getEventNumber()
            << ", NumPlanes: " << tbEvt.getNumPlanes()
            << ", TrigID: " << tbEvt.getTriggerId());

    for (size_t index = 0; index < tbEvt.getNumPlanes(); ++index) {
      const std::shared_ptr<const std::vector<TelDigit> > digits = tbEvt.getDigits(index);
      for (size_t iDigit = 0; iDigit < digits->size(); ++iDigit) {
        const TelDigit& digit = digits->at(iDigit);
        unsigned short planeNo = digit.getSensorID();
        std::map<unsigned short, VxdID>::const_iterator it = m_sensorID.find(planeNo);
        if (it == m_sensorID.end()) {
          // There must be a serious reason for this.
          B2ERROR("Incorrect plane number, unassociated with a VxdID " << planeNo);
          continue;
        } else {
          storeTelDigits.appendNew(it->second, digit.getUCellID(), digit.getVCellID(), 1.0);
        }
      }
    }
    storeTelEventInfo.assign(tbEvt.getTelEventInfo());
  } else {
    return false;
  }
  return true;
}

void TelDataMergerModule::initialize()
{
  B2DEBUG(75, "Initializing TelDataMergerModule...");

  // EventMetaData
  StoreObjPtr<EventMetaData> storeEventMetaData;
  storeEventMetaData.required();

  // FTSW is necessary, without it just fail.
  StoreArray<RawFTSW> storeFTSW("");
  storeFTSW.required();
  m_storeRawFTSWsName = storeFTSW.getName();

  // Register output data collections
  StoreArray<TelDigit> storeTelDigits(m_storeDigitsName);
  storeTelDigits.registerAsPersistent();
  m_storeDigitsName = storeTelDigits.getName();

  StoreObjPtr<TelEventInfo> storeTelEventInfo;
  storeTelEventInfo.registerAsPersistent();

  // Initialize sensor number to sensor VxdID conversion map.

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_sensorID.clear();
  unsigned short iPlane(0);
  for (VxdID layer : geo.getLayers(TEL::SensorInfo::TEL)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_sensorID.insert(std::pair<unsigned short, VxdID>(iPlane, sensor));
        iPlane++;
      }
    }
  }
  B2DEBUG(75, "TelDataMergerModule initialised!");
}

void TelDataMergerModule::beginRun()
{
  B2DEBUG(75, "Starting eudaq::FileReader...");

  // create data reader object
  m_reader = new eudaq::FileReader(m_inputFileName, "" , true);

  // if creation failed, fail with loud noise
  if (! m_reader) {
    B2FATAL("Creation of eudaq::FileReader object failed!");
    return;
  }

  // get run number of current run
  unsigned int runNo = m_reader->RunNumber();
  B2INFO("Operating on file \"" << m_reader->Filename() << "\", run Number is " << runNo);

  m_nDataEvents = 0;
  m_nBOREvents = 0;
  m_nEOREvents = 0;
  m_nNoTrigEvents = 0;

  // get current event
  const eudaq::Event& ev = m_reader->GetEvent();

  // check whether the resulting event is a BORE
  // if not, fail with loud noise
  if (! ev.IsBORE()) {
    B2FATAL("First event of run was not BORE! Quitting.");
    return;
  } else { // process BDRE
    processBOREvent(ev);
  }

  B2DEBUG(75, "Started eudaq::FileReader!");
}

void TelDataMergerModule::event()
{
  // if we've got here, we must have a valid reader and a data file starting with
  // BDRE.

  B2DEBUG(25, "Started Event();");

  StoreArray<RawFTSW> storeFTSW(m_storeRawFTSWsName);

  m_currentTLUTagFromFTSW =
    static_cast<unsigned short>(storeFTSW[0]->Get15bitTLUTag(0));
  timeval* currentTimeValFromFTSW(new timeval);
  storeFTSW[0]->GetTTTimeVal(0, currentTimeValFromFTSW);

  const eudaq::Event& ev = m_reader->GetEvent();

  if (ev.IsBORE()) {
    B2DEBUG(50, "Operating on BORE");
    processBOREvent(ev);
  } else if (ev.IsEORE()) {
    B2DEBUG(50, "Operating on EORE");
    processEOREvent(ev);
  } else {
    B2DEBUG(10, "Operating on normal event");
    if (getTelTriggerID(ev) >= 0) {
      // Here goes the search....
      // But now we only print out what's up and go on
      B2INFO("TLU tags: FTSW: " << m_currentTLUTagFromFTSW
             << " EUDAQ: " << m_currentTLUTagFromEUDAQ)
      processNormalEvent(ev);
    }
  }

  if (m_reader->NextEvent()) {
    B2DEBUG(25, "EUDAQ Reader returned good event.");
  } else {
    B2DEBUG(25, "EUDAQ Reader returned bad event.");
    stopPeacefully();
  }

  B2DEBUG(25, "Finished Event();");
  return;
}

void TelDataMergerModule::endRun()
{
  B2DEBUG(75, "Finishing run...");

  // delete data reader object
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2INFO("Processed " << m_nDataEvents << " data events!");
  if (! m_nBOREvents) {
    B2WARNING("No BORE found, possibly truncated file!");
  } else if (m_nBOREvents > 1) {
    B2WARNING("Multiple BOREs found!");
  }
  if (! m_nEOREvents) {
    B2WARNING("No EORE found, possibly truncated file!");
  } else if (m_nEOREvents > 1) {
    B2WARNING("Multiple EOREs found!");
  }

  if (m_nNoTrigEvents) {
    B2WARNING("Found " << m_nNoTrigEvents << " events without a valid trigger ID.");
  }

  B2DEBUG(75, "Finished run!");

}


void TelDataMergerModule::terminate()
{
  B2DEBUG(75, "Terminating TelDataMergerModule...");

  // just to be on the safe side
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2DEBUG(75, "TelDataMergerModule terminated!");
}
