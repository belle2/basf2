/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cstdint>

/* Belle2 headers. */
#include <bklm/dbobjects/BKLMElectronicMapping.h>
#include <klm/modules/KLMUnpacker/KLMUnpackerModule.h>
#include <klm/rawdata/RawData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(KLMUnpacker)

KLMUnpackerModule::KLMUnpackerModule() : Module(),
  m_triggerCTimeOfPreviousEvent(0)
{
  setDescription("EKLM unpacker (creates EKLMDigit from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputBKLMDigitsName", m_outputBKLMDigitsName,
           "Name of BKLMDigit store array.", string(""));
  addParam("outputEKLMDigitsName", m_outputEKLMDigitsName,
           "Name of EKLMDigit store array.", string(""));
  addParam("WriteWrongHits", m_WriteWrongHits,
           "Record wrong hits (e.g. for debugging).", false);
  addParam("IgnoreWrongHits", m_IgnoreWrongHits,
           "Ignore wrong hits (i.e. no B2ERROR).", false);
  addParam("IgnoreStrip0", m_IgnoreStrip0,
           "Ignore hits with strip = 0 (normally expected for certain firmware "
           "versions).", true);
  addParam("useDefaultModuleId", m_useDefaultModuleId,
           "Use default module id if not found in mapping.", true);
  addParam("keepEvenPackages", m_keepEvenPackages,
           "Keep packages that have even length normally indicating that "
           "data was corrupted ", false);
  addParam("SciThreshold", m_scintThreshold,
           "Scintillator strip hits with charge lower this value will be "
           "marked as bad.", double(140.0));
  addParam("loadThresholdFromDB", m_loadThresholdFromDB,
           "Load threshold from database (true) or not (false)", true);
  addParam("loadMapFromDB", m_loadMapFromDB,
           "Whether load electronic map from database", true);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMUnpackerModule::~KLMUnpackerModule()
{
}

void KLMUnpackerModule::initialize()
{
  /* Common. */
  m_RawKLMs.isRequired();
  /* BKLM. */
  m_bklmDigits.registerInDataStore(m_outputBKLMDigitsName);
  m_klmDigitRaws.registerInDataStore();
  m_bklmDigitOutOfRanges.registerInDataStore();
  m_DigitEventInfos.registerInDataStore();

  m_bklmDigits.registerRelationTo(m_klmDigitRaws);
  m_bklmDigitOutOfRanges.registerRelationTo(m_klmDigitRaws);
  m_DigitEventInfos.registerRelationTo(m_bklmDigits);
  m_DigitEventInfos.registerRelationTo(m_bklmDigitOutOfRanges);

  if (m_loadMapFromDB)
    loadMapFromDB();
  /* EKLM. */
  m_eklmDigits.registerInDataStore(m_outputEKLMDigitsName);
  m_DigitEventInfos.registerInDataStore();
  m_eklmDigits.registerRelationTo(m_DigitEventInfos);
}

void KLMUnpackerModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  if (!m_TimeConversion.isValid())
    B2FATAL("EKLM time conversion parameters are not available.");
  if (!m_Channels.isValid())
    B2FATAL("EKLM channel data are not available.");
  m_triggerCTimeOfPreviousEvent = 0;
}

void KLMUnpackerModule::unpackEKLMDigit(
  const int* rawData, EKLMDataConcentratorLane* lane,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  int endcap, layer, sector, strip = 0, stripGlobal;
  KLM::RawData raw;
  KLM::unpackRawData(rawData, &raw, nullptr, nullptr, false);
  /**
   * The possible values of the strip number in the raw data are
   * from 0 to 127, while the actual range of strip numbers is from
   * 1 to 75. A check is required. The unpacker continues to work
   * with B2ERROR because otherwise debugging is not possible.
   */
  bool correctHit = m_ElementNumbers->checkStrip(raw.channel, false);
  if (!correctHit) {
    if (!(m_IgnoreWrongHits ||
          (raw.channel == 0 && m_IgnoreStrip0))) {
      B2ERROR("Incorrect strip number in raw data."
              << LogVar("Strip number", raw.channel));
    }
    if (!m_WriteWrongHits)
      return;
    strip = raw.channel;
  } else {
    strip = m_ElementNumbers->getStripSoftwareByFirmware(
              raw.channel);
  }
  uint16_t plane = raw.axis + 1;
  /*
   * The possible values of the plane number in the raw data are from
   * 1 to 2. The range is the same as in the detector geometry.
   * Consequently, a check of the plane number is useless: it is
   * always correct.
   */
  lane->setLane(raw.lane);
  const int* sectorGlobal = m_ElectronicsMap->getSectorByLane(lane);
  if (sectorGlobal == nullptr) {
    if (!m_IgnoreWrongHits) {
      B2ERROR("Lane does not exist in the EKLM electronics map."
              << LogVar("Copper", lane->getCopper())
              << LogVar("Data concentrator", lane->getDataConcentrator())
              << LogVar("Lane", lane->getLane()));
    }
    if (!m_WriteWrongHits)
      return;
    endcap = 0;
    layer = 0;
    sector = 0;
    correctHit = false;
  } else {
    m_ElementNumbers->sectorNumberToElementNumbers(
      *sectorGlobal, &endcap, &layer, &sector);
  }
  EKLMDigit* eklmDigit = m_eklmDigits.appendNew();
  eklmDigit->addRelationTo(klmDigitEventInfo);
  eklmDigit->setCTime(raw.ctime);
  eklmDigit->setTDC(raw.tdc);
  eklmDigit->setTime(
    m_TimeConversion->getTime(raw.ctime, raw.tdc,
                              klmDigitEventInfo->getTriggerCTime(), true));
  eklmDigit->setEndcap(endcap);
  eklmDigit->setLayer(layer);
  eklmDigit->setSector(sector);
  eklmDigit->setPlane(plane);
  eklmDigit->setStrip(strip);
  eklmDigit->setCharge(raw.charge);
  if (correctHit) {
    stripGlobal = m_ElementNumbers->stripNumber(
                    endcap, layer, sector, plane, strip);
    const EKLMChannelData* channelData =
      m_Channels->getChannelData(stripGlobal);
    if (channelData == nullptr)
      B2FATAL("Incomplete EKLM channel data.");
    if (raw.charge < channelData->getThreshold())
      eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
    else
      eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
  }
}

void KLMUnpackerModule::unpackBKLMDigit(
  const int* rawData, int copper, int hslb,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  KLM::RawData raw;
  KLMDigitRaw* klmDigitRaw;
  KLM::unpackRawData(rawData, &raw, &m_klmDigitRaws, &klmDigitRaw, true);
  int electId = electCooToInt(copper - BKLM_ID, hslb,
                              raw.lane, raw.axis, raw.channel);
  int moduleId = 0;
  bool outRange = false;
  if (m_electIdToModuleId.find(electId) == m_electIdToModuleId.end()) {
    if (!m_useDefaultModuleId) {
      B2DEBUG(20, "KLMUnpackerModule:: could not find in mapping"
              << LogVar("Copper", copper)
              << LogVar("Finesse", hslb + 1)
              << LogVar("Lane", raw.lane)
              << LogVar("Axis", raw.axis));
      return;
    } else {
      moduleId = getDefaultModuleId(copper, hslb, raw.lane,
                                    raw.axis, raw.channel, outRange);
    }
  } else {
    // found moduleId in the mapping
    moduleId = m_electIdToModuleId[electId];

    // only channel and inRpc flag are not set yet
  }

  // moduleId counts are zero based
  int layer = (moduleId & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT;
  // int sector = (moduleId & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT;
  // int isForward = (moduleId & BKLM_END_MASK) >> BKLM_END_BIT;
  // int plane = (moduleId & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT;
  int channel = (moduleId & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT;

  if (layer > 14) {
    B2DEBUG(20, "KLMUnpackerModule:: strange that the layer number is larger than 14 "
            << LogVar("Layer", layer));
    return;
  }

  if (outRange) {
    // increase by 1 the event-counter of outOfRange-flagged hits
    klmDigitEventInfo->increaseOutOfRangeHits();

    // store the digit in the appropriate dataobject
    BKLMDigitOutOfRange* bklmDigitOutOfRange =
      m_bklmDigitOutOfRanges.appendNew(
        moduleId, raw.ctime, raw.tdc, raw.charge);
    bklmDigitOutOfRange->addRelationTo(klmDigitRaw);
    klmDigitEventInfo->addRelationTo(bklmDigitOutOfRange);

    std::string message = "channel number is out of range";
    m_rejected[message] += 1;
    m_rejectedCount++;
    B2DEBUG(21, "KLMUnpackerModule:: channel number is out of range"
            << LogVar("Channel", channel));
    return;
  }

  // still have to add channel and axis to moduleId
  if (layer > 1) {
    moduleId |= BKLM_INRPC_MASK;
    klmDigitEventInfo->increaseRPCHits();
  } else
    klmDigitEventInfo->increaseSciHits();
  // moduleId |= (((channel - 1) & BKLM_STRIP_MASK) << BKLM_STRIP_BIT) | (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);
  moduleId |= (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);

  BKLMDigit* bklmDigit =
    m_bklmDigits.appendNew(moduleId, raw.ctime, raw.tdc, raw.charge);
  bklmDigit->setTime(
    m_TimeConversion->getTime(raw.ctime, raw.tdc,
                              klmDigitEventInfo->getTriggerCTime(),
                              layer <= 1));
  if (layer < 2 && (raw.charge < m_scintThreshold))
    bklmDigit->isAboveThreshold(true);

  bklmDigit->addRelationTo(klmDigitRaw);
  klmDigitEventInfo->addRelationTo(bklmDigit);
}

void KLMUnpackerModule::event()
{
  /*
   * Length of one hit in 4-byte words. This is needed to find the hits in the
   * detector buffer.
   */
  const int hitLength = 2;
  EKLMDataConcentratorLane lane;
  for (int i = 0; i < m_RawKLMs.getEntries(); i++) {
    if (m_RawKLMs[i]->GetNumEvents() != 1) {
      B2ERROR("RawKLM a wrong number of entries (should be 1)."
              << LogVar("RawKLM index", i)
              << LogVar("Number of entries", m_RawKLMs[i]->GetNumEvents()));
      continue;
    }
    /*
     * getNumEntries is defined in RawDataBlock.h and gives the
     * numberOfNodes*numberOfEvents. Number of nodes is num copper boards.
     */
    for (int j = 0; j < m_RawKLMs[i]->GetNumEntries(); j++) {
      unsigned int copperId = m_RawKLMs[i]->GetNodeID(j);
      bool eklmHit = false;
      if ((copperId >= EKLM_ID) && (copperId <= EKLM_ID + 4))
        eklmHit = true;
      else if (!((copperId >= BKLM_ID) && (copperId <= BKLM_ID + 4)))
        continue;
      uint16_t copperN = copperId - EKLM_ID;
      lane.setCopper(copperN);
      m_RawKLMs[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        KLMDigitEventInfo* klmDigitEventInfo =
          m_DigitEventInfos.appendNew(m_RawKLMs[i], j);
        klmDigitEventInfo->setPreviousEventTriggerCTime(
          m_triggerCTimeOfPreviousEvent);
        m_triggerCTimeOfPreviousEvent = klmDigitEventInfo->getTriggerCTime();
        int numDetNwords = m_RawKLMs[i]->GetDetectorNwords(j, finesse_num);
        int* buf_slot    = m_RawKLMs[i]->GetDetectorBuffer(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        lane.setDataConcentrator(finesse_num);
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2ERROR("Incorrect number of data words."
                  << LogVar("Number of data words", numDetNwords));
          continue;
        }
        // in the last word there is the user word (from DCs)
        int userWord = (buf_slot[numDetNwords - 1] >> 16) & 0xFFFF;
        klmDigitEventInfo->setUserWord(userWord);
        for (int iHit = 0; iHit < numHits; iHit++) {
          if (eklmHit) {
            unpackEKLMDigit(&buf_slot[iHit * hitLength], &lane,
                            klmDigitEventInfo);
          } else {
            unpackBKLMDigit(&buf_slot[iHit * hitLength], copperId, finesse_num,
                            klmDigitEventInfo);
          }
        }
      }
    }
  }
}

void KLMUnpackerModule::endRun()
{
}

void KLMUnpackerModule::terminate()
{
}

void KLMUnpackerModule::loadMapFromDB()
{
  B2DEBUG(29, "KLMUnpackerModule:: reading from database...");

  DBArray<BKLMElectronicMapping> elements;
  for (const auto& element : elements) {
    B2DEBUG(29, "KLMUnpackerModule:: version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " <<
            element.getCopperId() <<
            ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
            ", isForward = " << element.getIsForward() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
            " plane(z/phi) = " << element.getPlane());

    int copperId = element.getCopperId();
    int slotId = element.getSlotId();
    int laneId = element.getLaneId();
    int axisId = element.getAxisId();
    int channelId = element.getChannelId();
    int sector = element.getSector();
    int isForward = element.getIsForward();
    int layer = element.getLayer();
    int plane =  element.getPlane();
    int stripId = element.getStripId();
    int elecId = electCooToInt(copperId - BKLM_ID, slotId - 1 , laneId, axisId, channelId);
    int moduleId = 0;

    moduleId = (isForward ? BKLM_END_MASK : 0)
               | ((sector - 1) << BKLM_SECTOR_BIT)
               | ((layer - 1) << BKLM_LAYER_BIT)
               | ((plane) << BKLM_PLANE_BIT)
               | ((stripId - 1) << BKLM_STRIP_BIT);
    m_electIdToModuleId[elecId] = moduleId;

    B2DEBUG(29, "KLMUnpackerModule:: electId: " << elecId << " moduleId: " << moduleId);
  }
}

int KLMUnpackerModule::electCooToInt(int copper, int finesse, int lane, int axis, int channel)
{
  // there are at most 16 coppers --> 4 bit
  // 4 finesse --> 2 bit
  // < 20 lanes --> 5 bit
  // axis --> 1 bit
  // channel --> 6 bit
  int ret = 0;
  copper = copper & 0xF;
  ret |= copper;
  finesse = finesse & 3;
  ret |= (finesse << 4);
  lane = lane & 0x1F;
  ret |= (lane << 6);
  axis = axis & 0x1;
  ret |= (axis << 11);
  channel = channel & 0x3F;
  ret |= (channel << 12);

  return ret;
}

/*void KLMUnpackerModule::intToElectCoo(int id, int& copper, int& finesse, int& lane)
{
  copper = 0;
  finesse = 0;
  lane = 0;
  copper = (id & 0xF);
  finesse = (id >> 4) & 3;
  lane = 0;
  lane = (id >> 6) & 0xF;
}*/

int KLMUnpackerModule::getDefaultModuleId(int copperId, int finesse, int lane, int axis, int channel, bool& outOfRange)
{
  int sector = 0;
  int isForward = 0;
  int layer = 0;
  int plane = 0;
  int stripId = 0;

  if (copperId == 117440513 || copperId == 117440514)
    isForward = 1;
  if (copperId == 117440515 || copperId == 117440516)
    isForward = 0;
  if (copperId == 117440513 || copperId == 117440515)
    sector = finesse + 3;
  if (copperId == 117440514 || copperId == 117440516)
    sector = (finesse + 7 > 8) ? finesse - 1 : finesse + 7;

  if (lane > 2)
    layer = lane - 5;
  else
    layer = lane;
  if (lane > 2)
    plane = axis;
  else {
    if (axis == 0)
      plane = 1;
    else
      plane = 0;
  }

  stripId =  getChannel(isForward, sector, layer, plane, channel);
  stripId =  flipChannel(isForward, sector, layer, plane, stripId, outOfRange);

  // attention: moduleId counts are zero based
  int moduleId = (isForward ? BKLM_END_MASK : 0)
                 | (uint(sector - 1) << BKLM_SECTOR_BIT)
                 | ((layer - 1) << BKLM_LAYER_BIT)
                 | ((plane) << BKLM_PLANE_BIT)
                 | ((stripId - 1) << BKLM_STRIP_BIT);

  return moduleId;
}

unsigned short KLMUnpackerModule::getChannel(int isForward, int sector, int layer, int axis, unsigned short channel)
{
  if (axis == 0 && layer < 3) { //scintillator z
    if (isForward == 0 && sector == 3) { //sector #3 is the top sector, backward sector#3 is the chimney sector.
      if (layer == 1) {
        if (channel > 8 && channel < 16)
          channel = 0;
        else if (channel > 0 && channel < 9)
          channel = 9 - channel;
        else if (channel > 30 && channel < 46)
          channel = 54 - channel;
        else if (channel > 15 && channel < 31)
          channel = 54 - channel;
      } else if (layer == 2) {
        if (channel == 16)
          channel = 0;
        else if (channel > 9 && channel < 16)
          channel = 0;
        else if (channel > 0 && channel < 10)
          channel = 10 - channel;
        else if (channel > 16 && channel < 31)
          channel = 40 - channel;
        else if (channel > 30 && channel < 46)
          channel = 69 - channel;
      }
    } else { //all sectors except backward sector#3
      if (channel > 0 && channel < 16)
        channel = 15 - channel + 1;
      else if (channel > 15 && channel < 31)
        channel = 45 - channel + 1;
      else if (channel > 30 && channel < 46)
        channel = 75 - channel + 1;
      else if (channel > 45 && channel < 61)
        channel = 105 - channel + 1;
    }
  }

  if (layer == 1) {
    if (axis == 1) { //phi strips
      if (channel > 0 && channel < 5)
        channel = 0;
      //else channel = channel - 4;
      if (channel > 4 && channel < 42)
        channel = channel - 4;
      //if(channel>41) channel=0;
      if (channel > 41)
        channel = channel - 4;
    }

    if (axis == 0 && !(isForward == 0 && sector == 3)) { //z strips
      //if (channel > 0 && channel < 10) channel = channel;
      if (channel > 0 && channel < 7)
        channel = 0;
      if (channel > 6 && channel < 61)
        channel = channel - 6;
      if (channel > 60)
        channel = channel - 6;
    }
  }
  if (layer == 2) {
    if (axis == 1) { //phi
      if (channel > 0 && channel < 3)
        channel = 0;
      if (channel > 2 && channel < 45)
        channel = channel - 2;
      if (channel > 44)
        channel = channel - 2;;
    }
    if (axis == 0 && !(isForward == 0 && sector == 3)) {
      //if (channel > 0 && channel < 10) channel = channel;
      if (channel > 0 && channel < 7)
        channel = 0;
      if (channel > 6 && channel < 61)
        channel = channel - 6;
      if (channel > 60)
        channel = channel - 6;;
    }

  }
  //if (layer > 2) channel = channel + 1;
  //if (sector == 3 && layer > 2 && layer < 16) channel = channel + 1;
  //if (sector == 7 && layer > 2 && layer < 16) channel = channel;

  return channel;
}

unsigned short KLMUnpackerModule::flipChannel(int isForward, int sector, int layer, int plane, unsigned short channel,
                                              bool& isOutRange)
{
  isOutRange = false;
  int MaxiChannel = 0;

  if (!isForward && sector == 3 && plane == 0) {
    if (layer < 3)
      MaxiChannel = 38;
    if (layer > 2)
      MaxiChannel = 34;
  } else {
    if (layer == 1 && plane == 1)
      MaxiChannel = 37;
    if (layer == 2 && plane == 1)
      MaxiChannel = 42;
    if (layer > 2 && layer < 7 && plane == 1)
      MaxiChannel = 36;
    if (layer > 6 && plane == 1)
      MaxiChannel = 48;
    //z plane
    if (layer == 1 && plane == 0)
      MaxiChannel = 54;
    if (layer == 2 && plane == 0)
      MaxiChannel = 54;
    if (layer > 2 && plane == 0)
      MaxiChannel = 48;
  }

  bool dontFlip = false;
  if (isForward && (sector == 7 ||  sector == 8 ||  sector == 1 ||  sector == 2))
    dontFlip = true;
  if (!isForward && (sector == 4 ||  sector == 5 ||  sector == 6 ||  sector == 7))
    dontFlip = true;
  if (!(dontFlip && layer > 2 && plane == 1) && (channel > 0 && channel < (MaxiChannel + 1)))
    channel = MaxiChannel - channel + 1;

  if (channel < 1 || channel > MaxiChannel)
    isOutRange = true;

  return channel;
}
