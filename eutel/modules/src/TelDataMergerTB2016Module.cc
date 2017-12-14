#include <eutel/modules/TelDataMergerTB2016Module.h>

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
REG_MODULE(TelDataMergerTB2016);

// file-local variables
namespace {
  // static const unsigned TLUID = eudaq::Event::str2id("_TLU");
  static const unsigned IDMASK = 0x7fff;
}

TelDataMergerTB2016Module::TelDataMergerTB2016Module() : Module(),
  m_bufferSize(100), m_nEventsProcess(-1), m_reader(NULL),
  m_nVXDDataEvents(0), m_nTelDataEvents(0), m_nEUDAQPXDDataEvents(0), m_nMapHits(0),
  m_nBOREvents(0), m_nEOREvents(0), m_nNoTrigEvents(0), m_update(true), m_PXDEUDAQNumber(11), m_mergeswitch(1),
  m_currentTLUTagFromFTSW(0), m_currentTimeStampFromFTSW(0), m_EUDAQTriggerOffset(1)
#ifdef debug_log
  , m_debugLog("TelReMerger.log")
#endif
{

  // Module Description
  setDescription("Data Merger Module for EUDET telescope data.");

  //Parameter definition
  addParam("inputFileName", m_inputFileName,
           "Input file name. For multiple files, use inputFileNames instead. Can be overridden using the -i argument to basf2.",
           std::string(""));
  addParam("storeTELDigitsName", m_storeTELDigitsName, "DataStore name of TelDigits collection", std::string("TelDigits_2"));
  addParam("storeOutputPXDDigitsName", m_storeOutput_PXDDigitsName, "DataStore name of Output PXDDigits collection",
           std::string("PXDDigits_2"));
  addParam("storeInputPXDDigitsName", m_storeInput_PXDDigitsName, "DataStore name of Input PXDDigits collection",
           std::string("PXDDigits"));
  addParam("bufferSize", m_bufferSize, "Size of the telescope data buffer", m_bufferSize);
  addParam("nEventsProcess", m_nEventsProcess, "Number of events to process", m_nEventsProcess);
  addParam("MergeSwitch", m_mergeswitch, "Switch to choose merging strategy: 1: trigger number, 2: PXD digit matching",
           m_mergeswitch);
  addParam("EUDAQTriggerOffset", m_EUDAQTriggerOffset,
           "Offset of the EUDAQ trigger number should be smaller than the SVD trigger number ", m_EUDAQTriggerOffset);



  // This is dirty, but safe for all practical purposes - there may be less EuTels, but hardly more.
  m_eutelPlaneNrs = {0, 1, 2, 3, 4, 5};
  addParam("eutelPlaneNrs", m_eutelPlaneNrs, "Numbering of eutel planes", m_eutelPlaneNrs);

  m_PXDPlaneNrs = {10, 11};
}


TelDataMergerTB2016Module::~TelDataMergerTB2016Module()
{
}

void TelDataMergerTB2016Module::stopPeacefully()
{
  StoreObjPtr<EventMetaData> storeEventMetaData;
  storeEventMetaData->setEndOfData();
}

bool TelDataMergerTB2016Module::processBOREvent(const eudaq::Event& ev)
{

  B2INFO("Processing BORE...");

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

bool TelDataMergerTB2016Module::processEOREvent(const eudaq::Event&)
{
  ++m_nEOREvents;
  if (m_nEOREvents > 1) {
    B2WARNING("Multiple EOREs (" << m_nEOREvents << ")");
  }
  return true;
}

void TelDataMergerTB2016Module::initialize()
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
  StoreArray<TelDigit> storeTelDigits(m_storeTELDigitsName);
  storeTelDigits.registerInDataStore();

  StoreArray<PXDDigit> storePXDDigits(m_storeOutput_PXDDigitsName);
  storePXDDigits.registerInDataStore();
  // If we choose the digit merging strategy we require a PXDDigit collection, else fail
  if (m_mergeswitch == 2) storePXDDigits.isRequired();

  StoreObjPtr<TelEventInfo> storeEventInfo;
  storeEventInfo.registerInDataStore();
  storeEventInfo.isRequired();

  if (m_nEventsProcess < 0) m_nEventsProcess = std::numeric_limits<long>::max();

  // Initialize sensor number to sensor VxdID conversion map.

  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  m_sensorID.clear();
  unsigned short iPlane(0);
  for (VxdID layer : geo.getLayers(TEL::SensorInfo::TEL)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {

        B2INFO("Eutel Plane No. " << m_eutelPlaneNrs[iPlane] << ", VXDID Sensor: " << sensor);
        m_sensorID.insert(std::make_pair(m_eutelPlaneNrs[iPlane], sensor));
        iPlane++;
      }
    }
  }


  // PXD sensor ID conversion between EUDAQ and VXD data stream
  m_PXDsensorID.clear();
  unsigned short iPlanePXD(0);
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {

        unsigned short layernumber = sensor.getLayerNumber();
        B2DEBUG(10, "Some Plane with index " << iPlanePXD << ", VXDID Sensor: " << sensor << " (getLayerNumber(): " << layernumber << ")");
        B2DEBUG(10, "Layer of this plane: " << layer);
        B2DEBUG(10, "Ladder of this plane: " << ladder);
        // PXD layers are 1 and 2
        if (layernumber == 1 || layernumber == 2) {
          m_PXDsensorID.insert(std::make_pair(m_PXDPlaneNrs[layernumber - 1], sensor));
          B2DEBUG(10, "This is a PXD layer! The EUDAQ ID for this sensor is " << m_PXDPlaneNrs[layernumber - 1]);
        }
        iPlanePXD++;
      }
    }
  }

  B2DEBUG(75, "TelDataMergerModule initialised!");
}

void TelDataMergerTB2016Module::beginRun()
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

  // Set the file reader for the buffer
  m_EUDAQEventBuffer.SetFileReader(m_reader);
  B2INFO(" File reader set for the EUDAQEventBuffer!");

  // Initialize the EUDAQEventBuffer
  B2INFO(" Inititalising the EUDAQEventBuffer...");
  m_EUDAQEventBuffer.SetBufferSize(m_bufferSize);
  B2INFO(" The buffer size is " << m_EUDAQEventBuffer.GetBufferSize());
  m_EUDAQEventBuffer.initialize();
  B2INFO(" EUDAQEventBuffer initialised!");

  B2DEBUG(75, "Started eudaq::FileReader!");
}



// Compare a single event via digit matching scheme
short int TelDataMergerTB2016Module::CompareDigitsSingleEvent(StoreArray<PXDDigit> VXDStreamPXDDigits,
    std::vector<short_PXDdigit_type> PXDdigitTuples)
{
  int num_matches = 0;

  //B2INFO("Size of the EUDAQ data Stream PXD Digit tuple: " << PXDdigitTuples.size());

  for (int iVXDPXD = 0; iVXDPXD < VXDStreamPXDDigits.getEntries(); iVXDPXD++) {

    // Get current  vxd data stream pxd digit column and row and ADC value
    PXDDigit* vxddigit = VXDStreamPXDDigits[iVXDPXD]; //get PXDDigits iPXD'th entry in StoreArray

    int vxd_col = vxddigit->getUCellID(); //get column
    int vxd_row = vxddigit->getVCellID(); //get row
    int vxd_sensorID = vxddigit->getSensorID(); //get sensor

    B2DEBUG(10, "\n VXD Stream PXD (ID: " << vxd_sensorID << ") Digit No.: " << iVXDPXD << ", (col row) = (" << vxd_col << " " <<
            vxd_row << ")" << "\n");

    int EUDAQDigitCounter = 0;

    for (int counter = 0; counter < PXDdigitTuples.size(); counter++) {

      // Get current eudaq data stream pxd digit column and row and ADC value

      int eudaq_col = -1;
      int eudaq_row = -1;

      // Check if this really is the PXD, 11 is the Sensor ID of the PXD Event in the EUDAQ data stream
      int EUDAQPXDSensorID = 11;
      if (std::get<0>(PXDdigitTuples.at(counter)) == EUDAQPXDSensorID) {
        eudaq_col = std::get<1>(PXDdigitTuples.at(counter)); //get column
        eudaq_row = std::get<2>(PXDdigitTuples.at(counter)); //get row

        B2DEBUG(10, "\n EUDAQ Stream PXD (SensorID: " << std::get<0>(PXDdigitTuples.at(counter)) << ") Digit No.: " << EUDAQDigitCounter <<
                ", (col row) = (" << eudaq_col << " " << eudaq_row << ")" << "\n");
      }


      if ((eudaq_col == vxd_col) && (eudaq_row == vxd_row)) {

        // If a match between digits is found say something, increase the number of matches and break the for loop
        // The break statement is necessary, because there are potential double digits in the eudaq pxd data stream
        B2DEBUG(10, "Digit Match between VXD PXD digit " << iVXDPXD << " and EUDAQ PXD digit " << EUDAQDigitCounter << "\n" <<
                "The EUDAQ pixel is at: (col row) = (" << eudaq_col << " " << eudaq_row << ")" << "\n");
        num_matches++;
        break;
      }
      EUDAQDigitCounter++;
    }
  }

  B2DEBUG(10, " " << num_matches << " (total possible: " << VXDStreamPXDDigits.getEntries() << ") PXD Digit matches found! \n");

  return num_matches;
}

void TelDataMergerTB2016Module::saveDigits(int matchposition)
{
  StoreArray<TelDigit> storeTELDigits2(m_storeTELDigitsName);
  StoreArray<PXDDigit> storePXDDigits2(m_storeOutput_PXDDigitsName);
  StoreObjPtr<TelEventInfo> storeTelEventInfo;

  B2INFO("Found a match for event " << m_currentTLUTagFromFTSW << ". Storing digits.");

  EUDAQEvent evt = m_EUDAQEventBuffer.GetEUDAQEvent(matchposition);
  //short int EUDAQTrigger=std::get<0>(evt);
  std::vector<short_TELdigit_type> TELDigits = std::get<1>(evt);
  std::vector<short_PXDdigit_type> PXDDigits = std::get<2>(evt);

  B2DEBUG(10, "TEL digits ");

  for (int counter = 0; counter < TELDigits.size(); counter++) {
    short_TELdigit_type teldigit = TELDigits.at(counter);
    unsigned short planeNo = std::get<0>(teldigit);
    auto it = m_sensorID.find(planeNo);
    if (it == m_sensorID.end()) {
      // There must be a serious reason for this.
      B2ERROR("Incorrect plane number, unassociated with a VxdID " << planeNo);
      continue;
    } else {

      B2DEBUG(10, "it->first: " << it->first << ", it->second: " << it->second);
      storeTELDigits2.appendNew(it->second, std::get<1>(teldigit), std::get<2>(teldigit), 1.0);
      if (counter == 0) B2DEBUG(10, "Storing " << TELDigits.size() << "digits for telescope digits");
    }
  }

  B2DEBUG(10, "PXD digits ");

  if (PXDDigits.size() > 1 || std::get<3>(PXDDigits.at(0)) > 0) {
    for (int counter = 0; counter < PXDDigits.size(); counter++) {
      short_PXDdigit_type pxddigit = PXDDigits.at(counter);
      auto it = m_PXDsensorID.find(m_PXDEUDAQNumber);
      if (it == m_PXDsensorID.end()) {
        // There must be a serious reason for this.
        B2ERROR("Incorrect plane number, unassociated with a VxdID " << m_PXDEUDAQNumber);
        continue;
      }
      B2DEBUG(10, "it->first: " << it->first << ", it->second: " << it->second);
      storePXDDigits2.appendNew(it->second, std::get<1>(pxddigit), std::get<2>(pxddigit), std::get<3>(pxddigit));
      if (counter == 0) B2DEBUG(10, "Storing " << PXDDigits.size() << "digits for the PXD with the sensor ID " << m_PXDEUDAQNumber);

    }
  }
  // First entry: Event, Second entry Trigger ID, third entry: timestamp
  storeTelEventInfo.assign(new TelEventInfo(m_currentTLUTagFromFTSW, m_currentTLUTagFromFTSW, m_currentTimeStampFromFTSW));

  m_nMapHits++;
}

void TelDataMergerTB2016Module::event()
{
  // if we've got here, we must have a valid reader and a data file starting with
  // BDRE.
  if (!m_reader) return;

  B2DEBUG(25, "Started Event();");


  StoreArray<RawFTSW> storeFTSW(m_storeRawFTSWsName);

  // Get EventMetaData and EventInfo datastores
  StoreObjPtr<EventMetaData> storeEventMetaData;

  if (long(storeEventMetaData->getEvent()) > m_nEventsProcess) stopPeacefully();

  m_currentTLUTagFromFTSW = static_cast<unsigned short>(storeFTSW[0]->Get15bitTLUTag(0));
  timeval* time = new timeval;
  storeFTSW[0]->GetTTTimeVal(0, time);
  m_currentTimeStampFromFTSW = time->tv_sec * 1000000 + time->tv_usec;

  B2INFO("\n ---------------------------\n" << "Starting new VXD stream Event!" << "\n This is the " <<
         m_nVXDDataEvents << "'th time the event() function has been used" << "\n ---------------------------");

  // Get Store array of the VXD data stream PXD digits
  StoreArray<PXDDigit> VXDStreamPXDDigits(m_storeInput_PXDDigitsName);
  const int nVXDDigits = VXDStreamPXDDigits.getEntries();


  // false if no digit matches are found for this event and the neighboring events defined by m_EventRange
  // if this variable isn't true after the digit matching processes a warning will be issued
  bool match = false;

  // Variable used to save the position of the matched event in the EUDAQEventBuffer
  int matchposition = -1;

  for (int counter = 0; counter < m_bufferSize; counter++) {
    if (m_mergeswitch == 2) {
      EUDAQEvent EUDAQ_evt = m_EUDAQEventBuffer.GetEUDAQEvent(counter);
      std::vector<short_PXDdigit_type> PXDDigits = std::get<2>(EUDAQ_evt);
      short int EUDAQTrigger = std::get<0>(EUDAQ_evt);
      // B2INFO("Buffer entry " << counter << " EUDAQ Event with trigger number " << EUDAQTrigger << " and " << PXDDigits.size() << " PXD digits" );

      // Check whether this buffer PXD Subevent contains the same digits as the VXD data stream PXD event
      short int num_digitmatches = CompareDigitsSingleEvent(VXDStreamPXDDigits, PXDDigits);

      if (num_digitmatches == nVXDDigits) {
        match = true;
        matchposition = counter;
        B2INFO("Digit match between VXD data stream PXD subevent with trigger number " << m_currentTLUTagFromFTSW <<
               " and EUDAQ data stream PXD subevent with trigger number " << EUDAQTrigger << "( " << num_digitmatches << " of " << nVXDDigits <<
               " found)");
        saveDigits(matchposition);
        break;
      }
    }

    else {
      EUDAQEvent EUDAQ_evt = m_EUDAQEventBuffer.GetEUDAQEvent(counter);
      short int EUDAQTrigger = std::get<0>(EUDAQ_evt);

      if ((EUDAQTrigger - m_EUDAQTriggerOffset) == m_currentTLUTagFromFTSW) {
        match = true;
        matchposition = counter;
        B2INFO("Match between VXD data stream trigger number " << m_currentTLUTagFromFTSW << " and Telescope trigger number " <<
               EUDAQTrigger);
        saveDigits(matchposition);
        break;
      }
    }

  }

  if (!match) {
    B2WARNING("No clear match between digits from VXD and EUDAQ data stream for VXD data stream event with Trigger: " <<
              m_currentTLUTagFromFTSW << "\n" <<
              "Also no matches for any other events in buffer!\n");
  } else {
    //Remove the matched event from the buffer and get the next event
    if (m_update) {
      m_update = m_EUDAQEventBuffer.Update(matchposition);
    }

    if (!m_update) {
      B2WARNING("Buffer update didn't work! No more updates from now on.");
    }
  }
  m_nVXDDataEvents++;
  B2DEBUG(25, "Finished VXD Event();");

  return;
}

void TelDataMergerTB2016Module::endRun()
{
  B2DEBUG(75, "Finishing run...");

  // delete data reader object
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2INFO("Processed " << m_nVXDDataEvents << " VXD data events!");
  B2INFO("Matched " << m_nMapHits << " Tel Data events.");

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


void TelDataMergerTB2016Module::terminate()
{
  B2DEBUG(75, "Terminating TelDataMergerModule...");

  // just to be on the safe side
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2DEBUG(75, "TelDataMergerModule terminated!");
}
