#include <eutel/modules/TelDataReaderModule.h>

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

#include <pxd/dataobjects/PXDDigit.h>

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
REG_MODULE(TelDataReader);


// file-local variables
namespace {
  // static const unsigned TLUID = eudaq::Event::str2id("_TLU");
  static const unsigned IDMASK = 0x7fff;
}

TelDataReaderModule::TelDataReaderModule() : Module(),
  m_counter(0),
  m_eventNo(0),
  m_runNo(0),
  m_fileID(0),
  m_reader(NULL),
  m_runNumbers()
{

  // Module Description
  setDescription("Data Reader Module for EUDET telescope data.");
  setPropertyFlags(c_Input);

  //Parameter definition
  std::vector<std::string> emptyStringVector;
  addParam("inputFileName", m_inputFileName,
           "Input file name. For multiple files, use inputFileNames instead. Can be overridden using the -i argument to basf2.",
           std::string(""));
  addParam("inputFileNames", m_inputFileNames,
           "List of input files. You may use wildcards to specify multiple files, e.g. 'somePrefix_*.root'. Can be overridden using the -i argument to basf2.",
           emptyStringVector);
  addParam("maxNumEntries", m_ullMaxNumEntries,
           "The Maximum number of enries to be processed.\nIf this is zero, no restrict will be placed.",
           static_cast<unsigned long long int>(0));
  addParam("numEntriesSkip", m_ullNumEntriesSkip, "The number of entries that shall be skipped at the beginning.",
           static_cast<unsigned long long int>(0));
  addParam("numEntriesPrint", m_numPrint,
           "How often to print the current entry number.\nIf this is zero, do not print any progress output.",
           static_cast<unsigned long long int>(0));
  // This is dirty, but safe for all practical purposes - there may be less EuTels, but hardly more.
  m_eutelPlaneNrs = {0, 1, 2, 3, 4, 5};
  addParam("eutelPlaneNrs", m_eutelPlaneNrs, "EUDAQ plane numbers ordered in beam direction", m_eutelPlaneNrs);
  // This is dirty, but safe for all practical purposes
  m_pxdPlaneNrs = {11};
  addParam("pxdPlaneNrs", m_pxdPlaneNrs, "PXD plane numbers ordered in beam direction", m_pxdPlaneNrs);
}


TelDataReaderModule::~TelDataReaderModule()
{
}


void TelDataReaderModule::initialize()
{
  B2DEBUG(75, "Initialising TelDataReaderModule...");


  const std::vector<std::string>& inputFiles = getInputFiles();
  if (inputFiles.empty()) {
    B2FATAL("You have to set either the 'inputFileName' or the 'inputFileNames' parameter, or start basf2 with the '-i MyFile.root' option.");
    return;
  }
  if (! m_inputFileName.empty() && ! m_inputFileNames.empty()) {
    B2FATAL("Cannot use both 'inputFileName' and 'inputFileNames' parameters!");
    return;
  }

  // we'll only use m_inputFileNames from now on
  m_inputFileNames = inputFiles;
  m_inputFileName = "";

  // register event meta data collection as persistent
  m_eventMetaDataPtr.registerInDataStore();

  // Register output data collections
  StoreArray<TelDigit> storeTelDigits("TelDigits");
  storeTelDigits.registerInDataStore();
  StoreObjPtr<TelEventInfo> storeTelEventInfo("TelEventInfo");
  storeTelEventInfo.registerInDataStore();
  StoreArray<PXDDigit> storePXDDigits("ePXDDigits");
  storePXDDigits.registerInDataStore();


  B2DEBUG(75, "TelDataReaderModule initialised!");
}

void TelDataReaderModule::start_run()
{
  B2DEBUG(75, "Starting new run...");

  // if there is no input file left -> goto terminate()
  if (m_fileID >= m_inputFileNames.size()) {
    B2DEBUG(25, "No input file(s) left -> terminating module.");
    TelDataReaderModule::terminate();
    return;
  }

  // create data reader object
  //   first argument: Input File Name
  //   second argument: <empty>
  //   third argument: resync flag
  m_reader = new eudaq::FileReader(m_inputFileNames.at(m_fileID), "" , false);

  // if creation failed -> goto terminate()
  if (! m_reader) {
    B2WARNING("Creation of eudaq::FileReader object failed!");
    TelDataReaderModule::terminate();
    return;
  }

  // get run number of current run
  m_runNo = m_reader->RunNumber();
  B2INFO("Operating on file \"" << m_reader->Filename() << "\", run Number is " << m_runNo);

  if (m_runNumbers.find(m_runNo) != m_runNumbers.end()) {
    B2WARNING("Run number " << m_runNo << " has already been processed. Will skip this input.");
    TelDataReaderModule::end_run();
    return;
  }
  m_runNumbers.insert(m_runNo);

  m_nData = 0;
  m_nDataLast = 0;
  m_nNonDet = 0;
  m_nBORE = 0;
  m_nEORE = 0;
  m_nNoTrig = 0;

  // Set sensor number to sensor VxdID conversion map. Must not be done in init,
  // because geometry must already be available.
  // ASSUMPTION: Planes are numbered from 0 to n in beam direction.

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

  m_PxdSensorID.clear();
  iPlane = 0;
  for (VxdID layer : geo.getLayers(TEL::SensorInfo::PXD)) {
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        m_PxdSensorID.insert(std::make_pair(m_pxdPlaneNrs[iPlane], sensor));
        iPlane++;
      }
    }
  }

  B2DEBUG(75, "Started new run!");
}

void TelDataReaderModule::event()
{

  B2DEBUG(25, "Started Event();");

  // we need to manually perform the start_run() / end_run() functions
  // => check whether m_reader pointer is set and execute start_run()
  // otherwise
  if (! m_reader) {
    B2DEBUG(25, "Calling start_run() to setup file reader.");
    TelDataReaderModule::start_run();
    B2DEBUG(25, "Finished calling start_run() to setup file reader.");

    // get current event
    const eudaq::Event& evTmp = m_reader->GetEvent();

    // check whether the resulting event is a BORE
    // each run MUST start with a BORE
    if (! evTmp.IsBORE()) {
      B2ERROR("First event of run was not BORE!");
      B2ERROR("Skipping this run!");
      TelDataReaderModule::end_run();
      return;
    }
  }

  // check whether we should skip some events at the beginning
  if (m_ullNumEntriesSkip > 0) {
    // If we are supposed to skip some events, do this by calling
    // repeatedly calling eudaq::FileReader::NextEvent().
    // Thus, we have to check whether the newly obtained event is
    // still valid after each call. If there are not sufficient
    // valid events to be skipped, issue an ERROR statement and skip
    // the run.
    unsigned long long int ctr = m_ullNumEntriesSkip;
    while (ctr > 0) {
      B2DEBUG(25, "Skipping forward one event.");
      if (! m_reader->NextEvent()) {
        B2ERROR("Number of events to be skipped exceeds number of valid events present!");
        B2ERROR("Skipping this run!");
        TelDataReaderModule::end_run();
        return;
      }
      --ctr;
    }

    // get current event
    const eudaq::Event& evTmp = m_reader->GetEvent();

    // The event we are pointing to now should not be an EORE
    // (otherwise we would process a run consisting of BORE and
    // EORE). If we skipped all data events, issue an error.
    if (evTmp.IsEORE()) {
      B2ERROR("Skipped all data events, now pointing to EORE.");
      B2ERROR("Will skip this run!");
      TelDataReaderModule::end_run();
      return;
    }
  }

  // setup variables for keeping track whether this was BORE or EORE
  bool bIsBORE = false;
  bool bIsEORE = false;

  // setup variables for keeping track whether processing of this run
  // should be stopped
  bool bAbortProcessing = false;
  bool bNoNextEvent = false;

  // Setup Belle2 Datastore
  StoreArray<TelDigit> storeTelDigits("TelDigits");
  StoreObjPtr<TelEventInfo> storeTelEventInfo("TelEventInfo");
  StoreArray<PXDDigit> storePXDDigits("ePXDDigits");

  const eudaq::Event& ev = m_reader->GetEvent();

  if (m_ullMaxNumEntries > 0 && m_counter >= m_ullMaxNumEntries) {
    B2DEBUG(50, " --> NumProcessedEntries:   " << m_counter << " ==> Exceeds MaxNumEntries ==> Abort.");
    bAbortProcessing = true;
  } else {
    B2DEBUG(10, " --> NumProcessedEntries:   " << m_counter);
  }

  if (m_numPrint && m_counter % m_numPrint == 0) {
    B2INFO("  ---> " << std::setw(10) << m_counter);
  }

  // shall we process this event?
  if (! bAbortProcessing) {

    // ========== Is Beginning of Run Event ========== //
    if (ev.IsBORE()) {
      B2DEBUG(50, "Operating on BORE");
      ++m_nBORE;
      if (m_nBORE > 1) {
        B2WARNING("Multiple BOREs (" << m_nBORE << ")");
      }

      // Process BORE event
      if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {
        // Initialize pugin manager
        eudaq::PluginManager::Initialize(* detEv);
      }

      bIsBORE = true;
    } // ========== Is End of Run Event ========== //
    else if (ev.IsEORE()) {
      B2DEBUG(50, "Operating on EORE");
      ++m_nEORE;
      if (m_nBORE == 0) {
        B2WARNING("No BORE found prior to EORE!");
      }
      if (m_nEORE > 1) {
        B2WARNING("Multiple EOREs (" << m_nEORE << ")");
      }

      bIsEORE = true;

    } // ========== Is Normal Event ========== //
    else {
      B2DEBUG(10, "Operating on normal event");
      if (m_nBORE == 0) {
        B2WARNING("No BORE found prior to data event!");
      }

      ++m_nData;

      if (const eudaq::DetectorEvent* detEv = dynamic_cast<const eudaq::DetectorEvent*>(& ev)) {

        // B2DEBUG(25, "##################################################");

        m_eventNo = detEv->GetEventNumber();

        unsigned int uiTrigID;
        bool bFoundTrigId = false;

        for (size_t i = 0; i < (*detEv).NumEvents(); ++i) {
          const eudaq::Event* subEv = (*detEv).GetEvent(i);

          // the first time we find a SubEvent which is of
          // type EUDRB or NI, we collect its Trigger ID and
          // assume its the trigger ID of the whole event
          if (std::string("EUDRB") == subEv->GetSubType() ||
              std::string("NI") == subEv->GetSubType()) {
            uiTrigID = (eudaq::PluginManager::GetTriggerID(* subEv) & IDMASK);
            bFoundTrigId = true;
            break;
          }
        }

        for (size_t i = 0; i < (*detEv).NumEvents(); ++i) {
          const eudaq::Event* subEv = (*detEv).GetEvent(i);
          B2DEBUG(15, "  TrigID  " << (eudaq::PluginManager::GetTriggerID(* subEv) & IDMASK)
                  << "  (" << subEv->GetSubType() << ")");
        }

        if (! bFoundTrigId) {
          ++m_nNoTrig;
          B2WARNING("No event of \"EUDRB\" or \"NI\" subtype found! \n Could not extract Trigger ID. \n Will skip this event.");
        }

        // it is really important to note here that we must
        // manully set the trigger ID after the conversion step
        // has finished. This is due to the fact that a certain
        // TLU ID Mask was used, which in turn is not considered
        // in the eudaq part.
        TBTelEvent tbEvt = eudaq::PluginManager::ConvertToTBTelEvent(* detEv);
        tbEvt.setTriggerId(uiTrigID);

        B2DEBUG(10, "TBEvent: Event: " << tbEvt.getEventNumber()
                << ", NumPlanes (Tel): " << tbEvt.getNumTelPlanes()
                << ", NumPlanes (PXD): " << tbEvt.getNumPXDPlanes()
                << ", TrigID: " << tbEvt.getTriggerId());

        // Store TelDigits
        for (size_t index = 0; index < tbEvt.getNumTelPlanes(); ++index) {
          const std::shared_ptr<const std::vector<TelDigit> > digits = tbEvt.getTelDigits(index);
          for (size_t iDigit = 0; iDigit < digits->size(); ++iDigit) {
            const TelDigit& digit = digits->at(iDigit);
            int planeNo = digit.getSensorID();
            auto it = m_sensorID.find(planeNo);
            if (it == m_sensorID.end()) {
              // There must be a serious reason for this.
              B2ERROR("Incorrect telescope plane number, unassociated with a VxdID " << planeNo);
              continue;
            } else {
              storeTelDigits.appendNew(it->second, digit.getUCellID(), digit.getVCellID(), 1.0);
            }
          }
        }

        // Store PXDDigits
        for (size_t index = 0; index < tbEvt.getNumPXDPlanes(); ++index) {
          const std::shared_ptr<const std::vector<PXDDigit> > digits = tbEvt.getPXDDigits(index);
          for (size_t iDigit = 0; iDigit < digits->size(); ++iDigit) {
            const PXDDigit& digit = digits->at(iDigit);
            int planeNo = digit.getSensorID();
            auto it = m_PxdSensorID.find(planeNo);
            if (it == m_PxdSensorID.end()) {
              // There must be a serious reason for this.
              B2ERROR("Incorrect PXD plane number, unassociated with a VxdID " << planeNo);
              continue;
            } else {
              storePXDDigits.appendNew(it->second,
                                       digit.getUCellID(),
                                       digit.getVCellID(),
                                       digit.getCharge());
            }
          }
        }

        // store telescope event info
        storeTelEventInfo.assign(tbEvt.getTelEventInfo());
      }

      // Update EventMetaData
      m_eventMetaDataPtr.create();
      m_eventMetaDataPtr->setExperiment(1);
      m_eventMetaDataPtr->setRun(m_runNo);
      m_eventMetaDataPtr->setEvent(m_eventNo);
    }
  }


  if (m_reader->NextEvent()) {
    B2DEBUG(25, "EUDAQ Reader returned good event.");
  } else {
    B2DEBUG(25, "EUDAQ Reader returned bad event.");
    bNoNextEvent = true;
  }

  // if this is a beginning of run event, recall the event function
  if (bIsBORE) {
    // if processing should be aborted at the same time, something
    // either went wrong with the max number of events to be
    // processed (bad input) or something else strange is going on
    // therefore, log this and jump to end_run();
    if (bAbortProcessing) {
      B2ERROR("Found Beginning of Run Event while processing should be aborted!");
      TelDataReaderModule::end_run();
      return;
    }
    // this event is BORE but data reader has not returned a valid
    // event => log this and jump to end_run();
    else if (bNoNextEvent) {
      B2ERROR("Found Beginning of Run Event but no valid data event present!");
      TelDataReaderModule::end_run();
      return;
    }
    // normal BORE case
    else {
      // do not increment entry counter because we operated on the
      // BORE, recall the event function in order to process the
      // first data event
      B2DEBUG(25, "Recalling event() for first data event.");
      TelDataReaderModule::event();
      B2DEBUG(25, "Finished recalling event() for first data event.");
      return;
    }
  }

  // if this is EORE set the end of data flag
  if (bIsEORE) {
    m_eventMetaDataPtr->setEndOfData();
  }

  // if there is a valid next event and this an EORE log it
  if (bIsEORE && ! bNoNextEvent) {
    B2WARNING("Found EORE while there's a valid next data event present.");
  }

  // if we are aorting processing but we did not reach the EORE log it
  if (bAbortProcessing && ! bIsEORE) {
    B2WARNING("Aborting processing prior to EORE because reached max entry limit.");
  }

  // if either
  //   a) this is EORE
  //   b) processing should be aborted because we reached the max
  //      number of events to be processed
  //   c) there is no valid next event
  // jump to end_run()
  if (bIsEORE || bAbortProcessing || bNoNextEvent) {
    TelDataReaderModule::end_run();
    return;
  }
  // increment the entry counter
  ++m_counter;

  B2DEBUG(25, "Finished Event();");
  return;
}

void TelDataReaderModule::end_run()
{
  B2DEBUG(75, "Finishing run...");

  // delete data reader object
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2INFO("Processed " << m_nData << " data events!");
  if (! m_nBORE) {
    B2WARNING("No BORE found, possibly truncated file!");
  } else if (m_nBORE > 1) {
    B2WARNING("Multiple BOREs found!");
  }
  if (! m_nEORE) {
    B2WARNING("No EORE found, possibly truncated file!");
  } else if (m_nEORE > 1) {
    B2WARNING("Multiple EOREs found!");
  }

  if (m_nNoTrig) {
    B2WARNING("Found " << m_nNoTrig << " events without a valid trigger ID.");
  }

  if (m_nNonDet || (m_nBORE != 1) || (m_nEORE > 1)) {
    B2ERROR("Probably corrupt file.");
  }

  B2DEBUG(75, "Finished run!");

  // increment file id counter and then goto start_run() to
  // check whether there's another data file to be processed
  ++m_fileID;
  TelDataReaderModule::start_run();
  return;
}


void TelDataReaderModule::terminate()
{
  B2DEBUG(75, "Terminating TelDataReaderModule...");

  // just to be on the safe side
  if (m_reader) { delete m_reader; m_reader = NULL; }

  B2DEBUG(75, "TelDataReaderModule terminated!");
}
