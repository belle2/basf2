#include <eutel/modules/TelDataMergerModule.h>

// include standard c++
#include <memory>
#include <algorithm>
#include <set>
#include <limits>

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
  m_bufferSize(100), m_nEventsProcess(-1), m_reader(NULL), m_buffer(m_bufferSize),
  m_bufferVXD(m_bufferSize),  m_nVXDDataEvents(0), m_nTelDataEvents(0), m_nMapHits(0),
  m_nBOREvents(0), m_nEOREvents(0), m_nNoTrigEvents(0), m_currentTLUTagFromFTSW(0),
  m_currentTimeStampFromFTSW(0), m_referenceTLUTag(-1),
  m_referenceTimeFromFTSW(0), m_referenceTimeFromEUDAQ(0)
#ifdef debug_log
  , m_debugLog("TelMerger.log")
#endif
{

  // Module Description
  setDescription("Data Merger Module for EUDET telescope data.");

  //Parameter definition
  addParam("inputFileName", m_inputFileName,
           "Input file name. For multiple files, use inputFileNames instead. Can be overridden using the -i argument to basf2.",
           std::string(""));
  addParam("storeDigitsName", m_storeDigitsName, "DataStore name of TelDigits collection", std::string(""));
  addParam("bufferSize", m_bufferSize, "Size of the telescope data buffer", m_bufferSize);
  addParam("nEventsProcess", m_nEventsProcess, "Number of events to process", m_nEventsProcess);

  // This is dirty, but safe for all practical purposes - there may be less EuTels, but hardly more.
  m_eutelPlaneNrs = {0, 1, 2, 3, 4, 5};
  addParam("eutelPlaneNrs", m_eutelPlaneNrs, "Numbering of eutel planes", m_eutelPlaneNrs);
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

short int TelDataMergerModule::getTLUTagFromEUDAQ(const eudaq::Event& ev)
{
  int currentTLUTagFromEUDAQ = -1;
  if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

    bool bFoundTrigId = false;
    for (size_t i = 0; i < (*detEv).NumEvents(); ++i) {
      const eudaq::Event* subEv = (*detEv).GetEvent(i);
      // the first time we find a SubEvent which is of type EUDRB or NI,
      // we collect its Trigger ID and assume it is the trigger ID
      // of the whole event NOPE.
      if (std::string("EUDRB") == subEv->GetSubType() ||
          std::string("NI") == subEv->GetSubType()) {
        currentTLUTagFromEUDAQ =
          static_cast<int>((eudaq::PluginManager::GetTriggerID(* subEv) & IDMASK));
        bFoundTrigId = true;
        break;
      }
    }

    if (! bFoundTrigId) {
      ++m_nNoTrigEvents;
      B2WARNING("No event of \"EUDRB\" subtype found! \n Could not extract Trigger ID. \n Will skip this event.");
    }
  }
  return currentTLUTagFromEUDAQ;
}

bool TelDataMergerModule::processNormalEvent(const eudaq::Event& ev)
{
  if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

    TBTelEvent tbEvt = eudaq::PluginManager::ConvertToTBTelEvent(* detEv);

    B2DEBUG(10, "TBEvent: Event: " << tbEvt.getEventNumber()
            << ", NumPlanes: " << tbEvt.getNumTelPlanes()
            << ", TrigID: " << tbEvt.getTriggerId());

    short int currentTLUTagFromEUDAQ = short(tbEvt.getEventNumber() % 32768);
    // it is really important to note here that we must manually set the trigger ID
    // after the conversion step has finished. This is due to the fact that a certain
    // TLU ID Mask was used, which in turn is not considered in the eudaq part.
    tbEvt.setTriggerId(currentTLUTagFromEUDAQ);

#ifdef debug_log
    m_debugLog << "TEL\tTelEvent\t" << tbEvt.getEventNumber() % 32768 << "\tTLU_tag\t" << currentTLUTagFromEUDAQ << "\ttimestamp\t" <<
               tbEvt.getTimeStamp() << std::endl;
#endif

    BoundedSpaceMap<eudaq_timestamp_type, short_digit_type>::collection_type digitTuples;
    for (size_t plane = 0; plane < tbEvt.getNumTelPlanes(); ++plane) {
      const std::shared_ptr<const std::vector<TelDigit> > digits = tbEvt.getTelDigits(plane);
      if (digits->size() == 0) return false;
      for (const TelDigit& digit : *digits) {
        short_digit_type dtuple =
          std::make_tuple(digit.getSensorID().getID(), digit.getUCellID(), digit.getVCellID());
        digitTuples.push_back(dtuple);
      }
    }
    m_buffer.put(currentTLUTagFromEUDAQ, tbEvt.getEventNumber(), digitTuples);
    ++m_nTelDataEvents;
  } else {
    return false;
  }
  return true;
}

bool TelDataMergerModule::addTelEventToBuffer()
{
  const eudaq::Event& ev = m_reader->GetEvent();

  bool result = false;
  if (ev.IsBORE()) {
    B2DEBUG(50, "Operating on BORE");
    processBOREvent(ev);
  } else if (ev.IsEORE()) {
    B2DEBUG(50, "Operating on EORE");
    processEOREvent(ev);
  } else {
    B2DEBUG(10, "Operating on normal event");
    if (processNormalEvent(ev)) result = true;
  }
  return result;
}

std::size_t TelDataMergerModule::advanceBuffer()
{
  std::size_t acquired(0);

  while (m_buffer.getFreeSize() > 0 || CIRC::compare(m_buffer.getMedian(), m_bufferVXD.getMedian())) {
    if (!m_reader->NextEvent()) {
      B2INFO("No more usable telescope events to acquire.");
      break;
    } else if (addTelEventToBuffer()) {
      acquired++;
    }
  }
  return acquired;
}


void TelDataMergerModule::initialize()
{
  B2DEBUG(75, "Initializing TelDataMergerModule...");

  // EventMetaData
  StoreObjPtr<EventMetaData> storeEventMetaData;
  storeEventMetaData.isRequired();

  // FTSW is necessary, without it just fail.
  StoreArray<RawFTSW> storeFTSW("");
  storeFTSW.isRequired();
  m_storeRawFTSWsName = storeFTSW.getName();

  // Register output data collections
  StoreArray<TelDigit> storeTelDigits(m_storeDigitsName);
  storeTelDigits.registerInDataStore();
  m_storeDigitsName = storeTelDigits.getName();

  StoreObjPtr<TelEventInfo> storeTelEventInfo;
  storeTelEventInfo.registerInDataStore();

  if (m_nEventsProcess < 0) m_nEventsProcess = std::numeric_limits<long>::max();

  // Initialize sensor number to sensor VxdID conversion map.

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_sensorID.clear();
  unsigned short iPlane(0);
  for (VxdID layer : geo.getLayers(TEL::SensorInfo::TEL)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_sensorID.insert(std::make_pair(m_eutelPlaneNrs[iPlane], sensor));
        iPlane++;
      }
    }
  }

  // Initialize buffer map
  m_buffer.setMaxSize(m_bufferSize);
  m_bufferVXD.setMaxSize(m_bufferSize);

  B2DEBUG(75, "TelDataMergerModule initialised!");
}

void TelDataMergerModule::beginRun()
{
  B2DEBUG(75, "Starting eudaq::FileReader...");

  if (m_inputFileName == "") {
    B2WARNING("No telescope data on input. Continuing silently.");
    m_reader = 0;
    return;
  }
  // create data reader object
  //   first argument: Input File Name
  //   second argument: <empty>
  //   third argument: resync flag
  m_reader = new eudaq::FileReader(m_inputFileName, "" , false);


  // if creation failed, make some noise but don't interfere
  if (! m_reader) {
    B2FATAL("Creation of eudaq::FileReader object failed!");
    return;
  }

  // get run number of current run
  unsigned int runNo = m_reader->RunNumber();
  B2INFO("Operating on file \"" << m_reader->Filename() << "\", run Number is " << runNo);

  m_nVXDDataEvents = 0;
  m_nTelDataEvents = 0;
  m_nMapHits = 0;
  m_nBOREvents = 0;
  m_nEOREvents = 0;
  m_nNoTrigEvents = 0;

  // get current event
  const eudaq::Event& ev = m_reader->GetEvent();

  // check whether the resulting event is a BORE
  // if not, complain, but do not fail.
  if (! ev.IsBORE()) {
    B2ERROR("First event of run was not BORE!.");
  } else { // process BDRE
    processBOREvent(ev);
  }

  B2DEBUG(75, "Started eudaq::FileReader!");
}

void TelDataMergerModule::saveDigits(tag_type currentTag)
{
  StoreArray<TelDigit> storeDigits(m_storeDigitsName);
  StoreObjPtr<TelEventInfo> storeTelEventInfo;

  B2INFO("Found a match for event " << currentTag << ". Storing digits.");

  auto digitTuples = m_buffer.getData(currentTag);

  for (auto dtuple : digitTuples) {
    int planeNo = std::get<0>(dtuple);
    auto it = m_sensorID.find(planeNo);
    if (it == m_sensorID.end()) {
      // There must be a serious reason for this.
      B2ERROR("Incorrect plane number, unassociated with a VxdID " << planeNo);
      continue;
    } else {
      storeDigits.appendNew(it->second, std::get<1>(dtuple), std::get<2>(dtuple), 1.0);
    }
  }
  storeTelEventInfo.assign(new TelEventInfo(m_currentTLUTagFromFTSW, m_currentTLUTagFromFTSW, m_currentTimeStampFromFTSW));
  m_nMapHits++;
}

void TelDataMergerModule::event()
{
  // if we've got here, we must have a valid reader and a data file starting with
  // BDRE.
  if (!m_reader) return;

  B2DEBUG(25, "Started Event();");

  StoreArray<RawFTSW> storeFTSW(m_storeRawFTSWsName);

  StoreObjPtr<EventMetaData> storeEventMetaData;

  if (long(storeEventMetaData->getEvent()) > m_nEventsProcess) stopPeacefully();

  m_currentTLUTagFromFTSW =
    static_cast<unsigned short>(storeFTSW[0]->Get15bitTLUTag(0));
  m_bufferVXD.put(m_currentTLUTagFromFTSW);
  timeval* time = new timeval;
  storeFTSW[0]->GetTTTimeVal(0, time);
  m_currentTimeStampFromFTSW = time->tv_sec * 1000000 + time->tv_usec;

  // advance buffer
  size_t advance = advanceBuffer();

  B2INFO("Event: VXD TLU: " << m_currentTLUTagFromFTSW <<
         " VXD mean:  " << m_bufferVXD.getMedian() <<
         " TEL mean: " << m_buffer.getMedian() <<
         " Advance:" << advance
        );

  if (m_buffer.hasKey(m_currentTLUTagFromFTSW)) {
    // we have a hit, do sanity check and then get the data and save digits.
    // Do we have valid reference data?
    size_t distance = CIRC::distance(m_referenceTLUTag, m_currentTLUTagFromFTSW);
    long long int eventTimeFromEUDAQ = m_buffer.getTimeStamp(m_currentTLUTagFromFTSW);
    long long int differenceEUTEL = eventTimeFromEUDAQ - m_referenceTimeFromEUDAQ;
    long long int differenceFTSW = (m_currentTimeStampFromFTSW - m_referenceTimeFromFTSW) / 2048;
    if (m_referenceTLUTag >= 0 && abs(differenceEUTEL - differenceFTSW) / distance > 32) {
      B2WARNING("Possible sync problem (ignore if only few isolated warnings):\n" << "Event: "
                << m_currentTLUTagFromFTSW << " "
                << "distance from previous: " << distance << " "
                << "difference: " << abs(differenceEUTEL - differenceFTSW)
               );
    } else {
      // only rebase and save if difference within bounds
      m_referenceTimeFromFTSW = m_currentTimeStampFromFTSW;
      m_referenceTimeFromEUDAQ = eventTimeFromEUDAQ;
      m_referenceTLUTag = m_currentTLUTagFromFTSW;
      saveDigits(m_currentTLUTagFromFTSW);
    }
#ifdef debug_log
    m_debugLog << "FTSW\tTLUtag\t" << m_currentTLUTagFromFTSW
               << "\ttimestamp\t" << m_currentTimeStampFromFTSW
               << "\tdifferenceEUTEL\t" << differenceEUTEL
               << "\tdifferenceFTSW\t" << differenceFTSW
               << "\tdifference\t" << differenceEUTEL - differenceFTSW
               << "\tdistance\t" << distance << std::endl;
#endif
  } else {
    // syncing failed, report
    B2WARNING("No match for VXD event with FTSW TLU tag " << m_currentTLUTagFromFTSW << std::endl
              << "If this appears frequently, try to increase buffer size.");
    if (!m_reader->NextEvent()) {
      B2DEBUG(25, "No more events from EUDAQ Reader.");
      stopPeacefully();
    }
  }

  m_nVXDDataEvents++;
  B2DEBUG(25, "Finished VXD Event();");
  return;
}

void TelDataMergerModule::endRun()
{
  B2DEBUG(75, "Finishing run...");

  // delete data reader object
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2INFO("Processed " << m_nVXDDataEvents << " VXD data events!");
  B2INFO("Matched " << m_nMapHits << "Tel Data events.");
  if (! m_nBOREvents) {
    B2INFO("No BORE found, possibly truncated file!");
  } else if (m_nBOREvents > 1) {
    B2INFO("Multiple BOREs found!");
  }
  if (! m_nEOREvents) {
    B2INFO("No EORE found, possibly truncated file!");
  } else if (m_nEOREvents > 1) {
    B2INFO("Multiple EOREs found!");
  }

  if (m_nNoTrigEvents) {
    B2INFO("Found " << m_nNoTrigEvents << " events without a valid trigger ID.");
  }

#ifdef debug_log
  m_debugLog.close();
#endif

  B2DEBUG(75, "Finished run!");

}


void TelDataMergerModule::terminate()
{
  B2DEBUG(75, "Terminating TelDataMergerModule...");

  // just to be on the safe side
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2DEBUG(75, "TelDataMergerModule terminated!");
}
