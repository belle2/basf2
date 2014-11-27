/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Giulia Casarosa, Eugenio Paoloni                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <svd/modules/svdUnpacker/SVDUnpackerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <arpa/inet.h>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::augmented_crc
#include <boost/cstdint.hpp>  // for boost::uint16_t
#define CRC16POLYREV 0x8005         // CRC-16 polynomial, normal representation 

#include <sstream>
#include <iomanip>
#include <cstring>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDUnpackerModule::SVDUnpackerModule() : Module()
{
  //Set module properties
  setDescription("Produce SVDDigits from RawSVD. NOTE: only zero-suppressed mode is currently supported!");
  //setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  addParam("svdDigitListName", m_svdDigitListName, "Name of the SVD Digits List", string(""));
  addParam("svdTransparentDigitListName", m_svdTransparentDigitListName, "Name of the SVD Digits List for transparent mode", string(""));
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", string(""));
  addParam("APVLatency", m_APVLatency, " APV latency (in ns)", float(0));
  addParam("APVSamplingTime", m_APVSamplingTime, " APV sampling time (in ns)", float(1));

  addParam("enableFineCMC", m_enableFineCMC, "Enable to use fine CMC for corrected ADC calculation", false);
  addParam("CMCGroupNr", m_nCmcGroup, "Number of groups for fine CMC calculation", short(4));


  addParam("noiseMapFileName", m_noiseMapFileName, "path+name of the noise file", string(""));

}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{

  StoreArray<RawSVD>::required(m_rawSVDListName);
  StoreArray<SVDDigit> storeDigits(m_svdDigitListName);
  storeDigits.registerInDataStore();
  StoreArray<SVDTransparentDigit> storeTransparentDigits(m_svdTransparentDigitListName);
  storeTransparentDigits.registerInDataStore();

  loadMap();
}

void SVDUnpackerModule::beginRun()
{

  //FTB Error Field
  m_f0 = 0;
  m_f3 = 0;
  m_f5 = 0;
  m_f6 = 0;
  m_f7 = 0;

  //failed checks
  m_failedChecks = 0;

  m_wrongFTBHeader = 0;
  m_wrongFADCTrailer = 0;
  m_wrongFADCcrc = 0;
  m_wrongFTBcrc = 0;
  m_badEvent = 0;
  m_wrongFTBtrailer = 0;

  m_noAPVHeader = 0;
  m_noNewDigit = 0;

  m_NewDigit = 0;

  m_runType = 0;

  m_noiseMap->initializeMap(m_noiseMapFileName);

}

void SVDUnpackerModule::event()
{

  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  //svdDigits.create();
  if (!svdDigits.isValid())
    svdDigits.create();
  else
    svdDigits.getPtr()->Clear();

  StoreArray<SVDTransparentDigit> svdTransparentDigits(m_svdTransparentDigitListName);
  if (!svdTransparentDigits.isValid())
    svdTransparentDigits.create();
  else
    svdTransparentDigits.getPtr()->Clear();

  if (! m_map) {
    B2ERROR("xml map not loaded, going to the next module");
    return;
  }

  for (int i = 0; i < rawSVDList.getEntries(); i++) {
    for (int j = 0; j < rawSVDList[ i ]->GetNumEntries(); j++) {

      int nWords = rawSVDList[i]->Get1stDetectorNwords(j);
      uint32_t* data32 = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j);

      B2DEBUG(1, "===== print data =====");
      printB2Debug(data32, data32, &data32[nWords - 1], nWords);
      B2DEBUG(1, "======================");

      if (sanityChecks(nWords, data32)) {
        switch (m_runType) {
          case 0x1 : // transparent mode
            fillSVDTransparentDigitList(nWords, data32, &svdTransparentDigits);
            break;
          case 0x2 : // zero-suppression mode
            fillSVDDigitList(nWords, data32, &svdDigits);
            break;
          default :
            break;
        }
      } else
        m_failedChecks++;
    }
  }

  if (svdTransparentDigits.getEntries() > 0) { // if transparent data exist
    emulateCmc(&svdTransparentDigits);
    emulateZeroSupp(&svdTransparentDigits, &svdDigits);
  }


  //  B2INFO("number of entries of the RawSVD list = "<< rawSVDList.getEntries());
  //  for (int i = 0; i < rawSVDList.getEntries(); i++)
  //    B2INFO(" number of entries of the current RawSVD object = "<< rawSVDList[ i ]->GetNumEntries())

  m_runType = 0;
}

void SVDUnpackerModule::endRun()
{

  B2INFO(" total failed checks = " << m_failedChecks);
  B2INFO("   m_wrongFTBHeader = " <<  m_wrongFTBHeader);
  B2INFO("   m_wrongFADCTrailer = " << m_wrongFADCTrailer);
  B2INFO("   m_wrongFADCcrc = " << m_wrongFADCcrc);
  B2INFO("   m_wrongFTBcrc = " << m_wrongFTBcrc);
  B2INFO("   m_badEvent = " << m_badEvent);
  B2INFO("   m_wrongFTBtrailer = " << m_wrongFTBtrailer);


  B2INFO(" m_noAPVHeader = " <<  m_noAPVHeader);

  B2INFO(" m_noNewDigit = " <<  m_noNewDigit);
  B2INFO(" m_NewDigit = " <<  m_NewDigit);

  B2INFO(" FTB Error Field");
  B2INFO("   m_f0 = " << m_f0);
  B2INFO("   m_f3 = " << m_f3);
  B2INFO("   m_f5 = " << m_f5);
  B2INFO("   m_f6 = " << m_f6);
  B2INFO("   m_f7 = " << m_f7);
}


void SVDUnpackerModule::terminate()
{
  delete m_map;
  delete m_noiseMap; m_noiseMap = NULL;
}

void SVDUnpackerModule::loadMap()
{

  //load the sensor MAP from xml file
  m_map = new SVDOnlineToOfflineMap(m_xmlMapFileName);

  m_noiseMap = new SVDStripNoiseMap(m_map);

}

bool SVDUnpackerModule::sanityChecks(int nWords, uint32_t* data32_in)
{

  //first of all verify FTB checksum, then proceed with FTB/FADC headers and trailers

  //read FTB Trailer
  struct FTBTrailer* theFTBTrailer = (struct FTBTrailer*) &data32_in[nWords - 1];
  if (theFTBTrailer->controlWord != 0xff55) {
    B2ERROR("WRONG FTB Trailer");
    m_wrongFTBtrailer++;
    //    printDebug(&data32_in[nWords - 1], data32_in, &data32_in[nWords-1], 4 );
    return false;
  } else
    B2DEBUG(1, "sanityChecks: FTB Trailer found");

  // verify FTB checksum
  if (! verifyFTBcrc(nWords - 1, data32_in, theFTBTrailer->crc16)) {
    B2ERROR("sanityChecks: FTB checksum NOT VERIFIED");
    m_wrongFTBcrc++;
    return false;
  } else
    B2DEBUG(1, "sanityChecks: FTB checksum verified");


  //read FTB Header:
  struct FTBHeader* theFTBHeader = (struct FTBHeader*)data32_in;
  if (theFTBHeader->controlWord != 0xffaa0000) {
    B2ERROR("WRONG FTB header format 0x" << std::hex << std::setw(8) << std::setfill('0') << theFTBHeader->controlWord);
    m_wrongFTBHeader++;
    //    printDebug(data32_in, data32_in, &data32_in[nWords-1], 4 );
    return false;
  } else
    B2DEBUG(1, "sanityChecks: FTB header format checked");

  //  unsigned int FTBEvtNum = theFTBHeader->eventNumber;

  //check FTB Errors Field
  if (((theFTBHeader->errorsField >> 4) & 0xf) != 0xf)
    B2ERROR("WRONG FTB Error Field format 0x" << std::hex << std::setw(8) << std::setfill('0') << theFTBHeader->errorsField)
    else {
      if (theFTBHeader->errorsField  == 0xf0)
        m_f0++;
      if (theFTBHeader->errorsField  == 0xf3)
        //  B2WARNING("(FTB Evt Num = TTD Evt Num ) != FADC Evt Num in FTB Evt Numb = " << FTBEvtNum);
        m_f3++;
      if (theFTBHeader->errorsField  == 0xf5)
        //  B2WARNING("(FTB Evt Num = FADC Evt Num ) != TTD Evt Num in FTB Evt Numb = " << FTBEvtNum);
        m_f5++;
      if (theFTBHeader->errorsField  == 0xf6)
        //  B2WARNING("(TTD Evt Num = FADC Evt Num ) != FTB Evt Num in FTB Evt Numb = " << FTBEvtNum);
        m_f6++;
      if (theFTBHeader->errorsField  == 0xf7)
        //  B2WARNING("TTD Evt Num != FADC Evt Num  != FTB Evt Num in FTB Evt Numb = " << FTBEvtNum);
        m_f7++;

    }

  //read FADC Main Header:
  struct MainHeader* theMainHeader = (struct MainHeader*) &data32_in[2];
  if (theMainHeader->check != 0x6) {
    B2ERROR("WRONG FADC main header format 0x" << std::hex << std::setw(8) << std::setfill('0') << theMainHeader->check);
    m_wrongFADCHeader++;
    //    printDebug(&data32_in[2], data32_in, &data32_in[nWords-1], 4 );
    return false;
  } else
    B2DEBUG(1, "FADC main header format checked");

  //check the run type
  m_runType = theMainHeader->runType;
  if (m_runType == 0x2) {
    B2DEBUG(1, "run type checked (zerosuppressed)");
  } else if (m_runType == 0x1) {
    B2DEBUG(1, "run type checked (transparent)");
  } else {
    B2ERROR("WRONG run type (expected = zero-suppressed or transparent), got 0x" << std::hex << std::setw(8) << std::setfill('0') << theMainHeader->runType);
    m_wrongRunType++;
    return false;
  }

  //read FADC Trailer
  struct FADCTrailer* theFADCTrailer = (struct FADCTrailer*) &data32_in[nWords - 2];
  if (theFADCTrailer->check != 0xe) {
    B2ERROR("WRONG FADC Trailer");
    m_wrongFADCTrailer++;
    //    printDebug(&data32_in[nWords - 2], data32_in, &data32_in[nWords-1], 4 );
    return false;
  } else
    B2DEBUG(1, "sanityChecks: FADC Trailer found");

  //check FTB Flags Field
  if (((theFADCTrailer->FTBFlags >> 5) & 0) != 0)
    B2ERROR("WRONG FTB Flags Field format 0x" << std::hex << std::setw(8) << std::setfill('0') << theFADCTrailer->FTBFlags)
    else  if ((theFADCTrailer->FTBFlags & 16) == 16) {
      //  B2WARNING(" FTB Flag: CRC error = 1");
      m_wrongFADCcrc++;
      return false;
    } else if ((theFADCTrailer->FTBFlags & 8) == 8) {
      //  B2WARNING(" FTB Flag: Bad Event = 1");
      m_badEvent++;
      return false;
    }

  return true;

}

bool SVDUnpackerModule::verifyFTBcrc(int nWords, uint32_t* data32_start, unsigned int crc16)
{

  //first swap all 32-bits word -> big endian
  uint32_t tmpBuffer[nWords];
  for (int i = 0; i < nWords; i++)
    tmpBuffer[i] = htonl(data32_start[i]);

  //  B2DEBUG(1,"FTB crc = "<<std::hex << std::setw(8) << std::setfill('0') << crc16);

  //compute crc
  boost::crc_basic<16> bcrc(0x8005, 0xffff, 0, false, false);
  bcrc.process_block(tmpBuffer, tmpBuffer + nWords);
  unsigned int checkCRC = bcrc.checksum();
  //  B2DEBUG(1,"OUR crc = "<<std::hex << std::setw(8) << std::setfill('0') << checkCRC);

  //check crc
  bool result = (checkCRC == crc16);

  return result;
}


void SVDUnpackerModule::fillSVDDigitList(int nWords, uint32_t* data32_in,  StoreArray<SVDDigit>* svdDigits)
{
  uint32_t* data32 = data32_in;

  data32 += 2;
  //re-read FADC main header
  struct MainHeader* theMainHeader = (struct MainHeader*)data32;

  data32++;

  //read APV Headers, data samples (if there), and FADC trailer:
  struct APVHeader* theAPVHeader = 0 ;
  struct data* aSample;
  struct data* previousSample;
  float time = -m_APVLatency; // latecy has to be subtracted from time
  int sample_index = 0;

  bool FADCtrailerFound = false;

  for (; !FADCtrailerFound  && (data32 != &data32_in[nWords]); ++data32) {

    if (((*data32 >> 30) & 0x3) == 0x2) { //APV header type

      time = -m_APVLatency;
      sample_index = 0;
      theAPVHeader = (struct APVHeader*) data32;
      //      B2INFO(" FADC NUMBER = "<< theMainHeader->FADCnum   <<"   APV NUMBER = "<<theAPVHeader->APVnum);
      B2DEBUG(1, "New APV header: 0x" << hex << *data32 << dec);

      continue;
    }

    if (((*data32 >> 31) & 0x1) == 0) {     //zero-suppressed data

      B2DEBUG(1, "ZS data: 0x" << hex << *data32 << dec);

      aSample = (struct data*) data32;
      previousSample = (struct data*)(data32 - 1);

      //if (aSample->stripNum != previousSample->stripNum)
      if (((*(data32 - 1) >> 30) & 0x3) == 0x2 || // previous data type is APV header
          aSample->stripNum != previousSample->stripNum) { // previous data has different strip number
        time = -m_APVLatency;
        sample_index = 0;
        if (svdDigits->getEntries() - 1 >= 0) {
          (*svdDigits)[svdDigits->getEntries() - 1]->setNextID(-1);
        }
      }


      //add the 3 data samples:
      if (theAPVHeader)  {
        for (int i = 0; i < 3; i++) {
          //SVDDigit* newDigit = m_map->NewDigit(theMainHeader->FADCnum, theAPVHeader->APVnum, aSample->stripNum, aSample->sample[i], int(time));
          SVDDigit* newDigit = m_map->NewDigit(theMainHeader->FADCnum, theAPVHeader->APVnum, aSample->stripNum, aSample->sample[i], sample_index);
          newDigit->setTime(time);

          if (time > -m_APVLatency + 5 * m_APVSamplingTime) {
            B2WARNING(" More than 6 consecutive (" << time << ") data samples associate to the same strip in FADC evt number = " << theMainHeader->trgNumber);
            continue;
          }

          time += m_APVSamplingTime;
          sample_index++;

          // Translation can return 0, if wrong FADC/APV combination is encountered.
          if (!newDigit) {
            m_noNewDigit++;
            continue;
          } else {
            B2DEBUG(2, newDigit->print());
            short current_id = svdDigits->getEntries();
            short current_index = newDigit->getIndex();
            if (current_index == 0) {
              newDigit->setPrevID(-1);
              newDigit->setNextID(current_id + 1); // if the sample is the last one in the strip, this value is overwritten to -1 in above lines
            } else {
              newDigit->setPrevID(current_id - 1);
              newDigit->setNextID(current_id + 1);
            }
            svdDigits->appendNew(*newDigit);
            m_NewDigit++;
            delete newDigit;
          }

        }//for (int i = 0; i < 3; i++) {
      } else {
        m_noAPVHeader++;
        B2WARNING(" FADC data before a valid APV header 0x" << std::hex << std::setw(8) << std::setfill('0') << *data32);
        //  printDebug(data32, data32_in, &data32_in[nWords-1], 4 );
      }
      continue;
    }


    if (((*data32 >> 28) & 0xf) == 0xe) { // FADC trailer type

      B2DEBUG(1, "FADC Trailer found");
      FADCtrailerFound = true;
      if (svdDigits->getEntries() - 1 >= 0) {
        (*svdDigits)[svdDigits->getEntries() - 1]->setNextID(-1);
      }

      continue;
    }

    B2WARNING("unknown data field, highest four bits: 0x" << std::hex << std::setw(8) << std::setfill('0') << *data32 << " data check bit is = " << (*data32 >> 31));
    //    printDebug(data32, data32_in, &data32_in[nWords-1], 4 );

    return;
  }

  if (&data32_in[nWords] != &data32[1])
    B2WARNING("FADC trailer appeared too early, data short by " << &data32_in[nWords] - &data32[1] << " bytes");



}

void SVDUnpackerModule::fillSVDTransparentDigitList(int nWords, uint32_t* data32_in,  StoreArray<SVDTransparentDigit>* svdTransparentDigits)
{

  uint32_t* data32 = data32_in;

  data32 += 2;
  //re-read FADC main header
  struct MainHeader* theMainHeader = (struct MainHeader*)data32;
  const unsigned short fadcID = theMainHeader->FADCnum;

  data32++;

  //read APV Headers, data samples (if there), and FADC trailer:
  struct tp_APVHeader* theAPVHeader = 0 ;
  struct tp_data* aSample;
  //float time = -m_APVLatency; // latecy has to be subtracted from time

  bool FADCtrailerFound = false;

  short curAPVnum = -1;
  short oldAPVnum = -1;
  unsigned short curAdcSample = 0;
  unsigned short oldAdcSample = 0;
  unsigned short APVchannel = 0;

  unsigned short doneSample = 0x0;

  SVDTransparentDigit* newDigits = NULL;

  for (; !FADCtrailerFound  && (data32 != &data32_in[nWords]); ++data32) {

    if (((*data32 >> 30) & 0x3) == 0x2) { //APV header type

      //time = -m_APVLatency;
      theAPVHeader = (struct tp_APVHeader*) data32;
      //      B2INFO(" FADC NUMBER = " << fadcID  << "   APV NUMBER = "<<theAPVHeader->APVnum);
      B2DEBUG(1, "Found APV header");

      oldAPVnum = curAPVnum;
      curAPVnum = theAPVHeader->APVnum;

      oldAdcSample = curAdcSample;
      curAdcSample = theAPVHeader->sample;

      APVchannel = 0;

      //create new 128 channel SVDTransparentDigit
      if (curAPVnum != oldAPVnum)  {

        B2DEBUG(1, "Found new APV header");

        if (oldAPVnum >= 0 && newDigits != NULL) {
          B2DEBUG(2, newDigits->print());
          newDigits->setNSample(oldAdcSample + 1);
          svdTransparentDigits->appendNew(*newDigits);
          delete newDigits;
          newDigits = NULL;
        }//if (oldAPVnum>0) {

        const SVDOnlineToOfflineMap::ChipInfo& chip_info =
          m_map->getChipInfo(fadcID, curAPVnum);
        const VxdID vxd_id   = chip_info.m_sensorID;
        const bool  is_u     = chip_info.m_uSide;

        newDigits = new SVDTransparentDigit(vxd_id, is_u, fadcID, curAPVnum, 0, NULL, NULL, NULL, NULL);

        doneSample = 0x0;

      }//if (curAPVnum!=oldAPVnum)  {

      continue;
    }//if (((*data32 >> 30) & 0x3) == 0x2) { //APV header type


    if (((*data32 >> 28) & 0xf) == 0xe) { // FADC trailer type

      B2DEBUG(1, "FADC Trailer found");

      oldAdcSample = curAdcSample;

      if (oldAPVnum >= 0 && newDigits != NULL) {
        B2DEBUG(2, newDigits->print());
        newDigits->setNSample(oldAdcSample + 1);
        svdTransparentDigits->appendNew(*newDigits);
        delete newDigits;
        newDigits = NULL;
      }//if (oldAPVnum>0) {

      FADCtrailerFound = true;

      continue;
    }

    if (((*data32 >> 31) & 0x1) == 0) {     //transparent data

      //B2DEBUG(1, "FADC data found (channel: " << APVchannel << ")");

      if (APVchannel >= 128) {
        B2ERROR("APVchannel (" << APVchannel << ") is unexpectedly over 128.");
        B2ERROR("Transparent data decode is aborted.");
        return;
      }

      if (APVchannel < 128) {

        aSample = (struct tp_data*) data32;

        newDigits->setADC(aSample->adc_1, APVchannel, curAdcSample);
        APVchannel++;

        newDigits->setADC(aSample->adc_2, APVchannel, curAdcSample);
        APVchannel++;

        if (APVchannel >= 128) {
          doneSample += (0x1 << curAdcSample);
        }//if(APVchannel==128) {

      }//if(0<=APVchannel&&APVchannel<128) {

      continue;
    }//if (((*data32 >> 31) & 0x1) == 0) {     //transparent data

    B2WARNING("unknown data field, highest four bits: 0x" << std::hex << std::setw(8) << std::setfill('0') << *data32 << " data check bit is = " << (*data32 >> 31));
    //    printDebug(data32, data32_in, &data32_in[nWords-1], 4 );

    return;
  }

  if (&data32_in[nWords] != &data32[1])
    B2WARNING("FADC trailer appeared too early, data short by " << &data32_in[nWords] - &data32[1] << " bytes");

  return;
}

void SVDUnpackerModule::emulateCmc(StoreArray<SVDTransparentDigit>* svdTransparentDigits)
{

  SVDTransparentDigit* tp_digit;

  int  pedestal [128] = {0};
  int  threshold[128] = {0};
  bool is_good  [128];

  const int nTpDigits = svdTransparentDigits->getEntries();
  for (int index = 0; index < nTpDigits; index++) {
    tp_digit = (*svdTransparentDigits)[index];

    const VxdID vxd_id  = tp_digit->getSensorID();
    const unsigned short fadc_id = tp_digit->getFadcID();
    const unsigned short apv_id  = tp_digit->getApvID();
    const unsigned short nSample = tp_digit->getNSample();

    const SVDOnlineToOfflineMap::ChipInfo& chip_info =
      m_map->getChipInfo(fadc_id, apv_id);
    const bool  is_u     = chip_info.m_uSide;
    const short channel0 = chip_info.m_channel0;
    const short parallel = (chip_info.m_parallel) ? 1 : -1;

    for (int channel_i = 0; channel_i < 128; channel_i++) {
      const unsigned short strip =
        channel0 + ((unsigned short)channel_i) * parallel;

      pedestal [channel_i] = (int)(m_noiseMap->getPedestal(vxd_id, is_u, strip));
      threshold[channel_i] = (int)(m_noiseMap->getThreshold(vxd_id, is_u,  strip));
      is_good  [channel_i] =       m_noiseMap->isGood(vxd_id, is_u,  strip);

      B2DEBUG(1, "VxdID: " << vxd_id.getID() << ", strip: " << strip << " (FADC :" << fadc_id << ", apv_id: " << apv_id << ") ped: " << pedestal[channel_i] << ", thre: " << threshold[channel_i]);
    }//for(int channel_i=0;channel_i<128;channel_i++) {

    /*********************************************************/
    /*** Common-mode correction calculation                ***/
    /*********************************************************/
    short cmc1[6] = {0};
    short cmc2[6] = {0};
    for (int sample_i = 0; sample_i < nSample; sample_i++) {

      /**********************************************/
      /*** 1st common-mode correction calculation ***/
      /**********************************************/
      short cmc1_sum = 0;
      short ncmc1 = 0;
      for (int channel_i = 0; channel_i < 128; channel_i++) {
        if (!is_good[channel_i]) continue;
        short adc = tp_digit->getADC(channel_i, sample_i);
        cmc1_sum += adc - pedestal[channel_i];
        ncmc1++;
      }//for(int channel_i=0;channel_i<128;channel_i++) {
      cmc1[sample_i] = (ncmc1 > 0) ? cmc1_sum / ncmc1 : 0;
      if (cmc1_sum < 0 && ncmc1 > 0) {
        // this operation must be done because (short)(-0.1) = 0
        // (of course, (short)(+0.1) = 0 ), but we want to get
        // (short)(-0.1) = -1.
        cmc1[sample_i] -= 1;
      }

      /**********************************************/
      /*** 2nd common-mode correction calculation ***/
      /**********************************************/
      short cmc2_sum = 0;
      short ncmc2 = 0;
      for (int channel_i = 0; channel_i < 128; channel_i++) {
        if (!is_good[channel_i]) continue;
        short adc = tp_digit->getADC(channel_i, sample_i);
        short corradc = adc - pedestal[channel_i] - cmc1[sample_i];
        if (-threshold[channel_i] <= corradc && corradc <= threshold[channel_i]) {
          cmc2_sum += corradc;
          ncmc2++;
        }
      }//for(int channel_i=0;channel_i<128;channel_i++) {
      cmc2[sample_i] = (ncmc2 > 0) ? cmc2_sum / ncmc2 : 0;
      if (cmc2_sum < 0 && ncmc2 > 0) {
        // this operation must be done because (short)(-0.1) = 0
        // (of course, (short)(+0.1) = 0 ), but we want to get
        // (short)(-0.1) = -1.
        cmc2[sample_i] -= 1;
      }

      //if(cmc1[sample_i]==0&&fadc_id==129&&apv_id==32) {
      //  B2INFO("CMC1_SUM: " << cmc1_sum << ", NCMC1: " << ncmc1);
      //}

    }// for(int sample_i=0;sample_i<nSample;sample_i++) {

    tp_digit->setCMC1(cmc1);
    tp_digit->setCMC2(cmc2);

    /*********************************************************/
    /*** Fine common-mode correction calculation           ***/
    /*********************************************************/
    short fine_cmc1[128][6] = {{0}};
    short fine_cmc2[128][6] = {{0}};
    if (m_nCmcGroup <= 0 || 128 % m_nCmcGroup != 0) {
      B2FATAL("Invalid CMCGroupNr (" << m_nCmcGroup << "). It must be a divisor of 128. (e.g. 1, 2, 4, 8, ...)");
    }
    const int nCmcGroup   = m_nCmcGroup;
    const int nCmcChannel = 128 / nCmcGroup;
    for (int sample_i = 0; sample_i < nSample; sample_i++) {

      /***************************************************/
      /*** 1st fine common-mode correction calculation ***/
      /***************************************************/
      for (int cmc_group_i = 0; cmc_group_i < nCmcGroup; cmc_group_i++) {
        short cmc1_sum = 0;
        short ncmc1 = 0;
        for (int cmc_channel_i = 0; cmc_channel_i < nCmcChannel; cmc_channel_i++) {
          int channel_i = nCmcChannel * cmc_group_i + cmc_channel_i;
          if (!is_good[channel_i]) continue;
          short adc = tp_digit->getADC(channel_i, sample_i);
          cmc1_sum += adc - pedestal[channel_i];
          ncmc1++;
        }//for(int cmc_channel_i=0;cmc_channel_i<nCmcChannel;cmc_channel_i++) {
        short tmp_fine_cmc1 = (ncmc1 > 0) ? cmc1_sum / ncmc1 : 0;
        if (cmc1_sum < 0 && ncmc1 > 0) {
          // this operation must be done because (short)(-0.1) = 0
          // (of course, (short)(+0.1) = 0 ), but we want to get
          // (short)(-0.1) = -1.
          tmp_fine_cmc1 -= 1;
        }
        for (int cmc_channel_i = 0; cmc_channel_i < nCmcChannel; cmc_channel_i++) {
          int channel_i = nCmcChannel * cmc_group_i + cmc_channel_i;
          fine_cmc1[channel_i][sample_i] = tmp_fine_cmc1;
        }
      }//for(int cmc_group_i=0;cmc_group_i<nCmcGroup;cmc_group_i++) {

      /***************************************************/
      /*** 2nd fine common-mode correction calculation ***/
      /***************************************************/
      for (int cmc_group_i = 0; cmc_group_i < nCmcGroup; cmc_group_i++) {
        short cmc2_sum = 0;
        short ncmc2 = 0;
        for (int cmc_channel_i = 0; cmc_channel_i < nCmcChannel; cmc_channel_i++) {
          int channel_i = nCmcChannel * cmc_group_i + cmc_channel_i;
          if (!is_good[channel_i]) continue;
          short adc = tp_digit->getADC(channel_i, sample_i);
          short corradc = adc - pedestal[channel_i] - fine_cmc1[channel_i][sample_i];
          if (-threshold[channel_i] <= corradc && corradc <= threshold[channel_i]) {
            cmc2_sum += corradc;
            ncmc2++;
          }
        }//for(int cmc_channel_i=0;cmc_channel_i<nCmcChannel;cmc_channel_i++) {
        short tmp_fine_cmc2 = (ncmc2 > 0) ? cmc2_sum / ncmc2 : 0;
        if (cmc2_sum < 0 && ncmc2 > 0) {
          // this operation must be done because (short)(-0.1) = 0
          // (of course, (short)(+0.1) = 0 ), but we want to get
          // (short)(-0.1) = -1.
          tmp_fine_cmc2 -= 1;
        }
        for (int cmc_channel_i = 0; cmc_channel_i < nCmcChannel; cmc_channel_i++) {
          int channel_i = nCmcChannel * cmc_group_i + cmc_channel_i;
          fine_cmc2[channel_i][sample_i] = tmp_fine_cmc2;
        }
      }//for(int cmc_group_i=0;cmc_group_i<nCmcGroup;cmc_group_i++) {

      //if(cmc1[sample_i]==0&&fadc_id==129&&apv_id==32) {
      //  B2INFO("CMC1_SUM: " << cmc1_sum << ", NCMC1: " << ncmc1);
      //}

    }// for(int sample_i=0;sample_i<nSample;sample_i++) {

    for (int sample_i = 0; sample_i < nSample; sample_i++) {
      for (int channel_i = 0; channel_i < 128; channel_i++) {
        tp_digit->setFineCMC1(fine_cmc1[channel_i][sample_i], channel_i, sample_i);
        tp_digit->setFineCMC2(fine_cmc2[channel_i][sample_i], channel_i, sample_i);
      }
    }

    /*************************************************************/
    /*** Set corrected ADC value                               ***/
    /*************************************************************/
    for (int sample_i = 0; sample_i < nSample; sample_i++) {
      tp_digit->setCMC(cmc1[sample_i] + cmc2[sample_i], sample_i);

      for (int channel_i = 0; channel_i < 128; channel_i++) {
        tp_digit->setFineCMC(fine_cmc1[channel_i][sample_i] + fine_cmc2[channel_i][sample_i], channel_i, sample_i);

        short corradc      = -9999;
        short fine_corradc = -9999;
        if (is_good[channel_i]) {
          short adc = tp_digit->getADC(channel_i, sample_i);
          corradc =
            adc - pedestal[channel_i] - cmc1[sample_i] - cmc2[sample_i];
          fine_corradc =
            adc - pedestal[channel_i] - fine_cmc1[channel_i][sample_i] - fine_cmc2[channel_i][sample_i];
        }
        tp_digit->setCorrADC(corradc     , channel_i, sample_i);
        tp_digit->setFineCorrADC(fine_corradc, channel_i, sample_i);

      }//for(int channel_i=0;channel_i<128;channel_i++) {

    }//for(int sample_i=0;sample_i<nSample;sample_i++) {

  }//for(int index=0;index<nTpDigits;index++) {

  return;
}

void SVDUnpackerModule::emulateZeroSupp(StoreArray<SVDTransparentDigit>* svdTransparentDigits, StoreArray<SVDDigit>* svdDigits)
{

  B2INFO("start emulateZeroSupp");

  SVDTransparentDigit* tp_digit;
  const int nTpDigits = svdTransparentDigits->getEntries();
  for (int index = 0; index < nTpDigits; index++) {
    tp_digit = (*svdTransparentDigits)[index];

    const VxdID vxd_id  = tp_digit->getSensorID();
    const unsigned short fadc_id = tp_digit->getFadcID();
    const unsigned short apv_id  = tp_digit->getApvID();
    const unsigned short nSample = tp_digit->getNSample();

    B2INFO("Index (" << index << ") FADC: " << fadc_id << ", APV: " << apv_id << ", NSample: " << nSample);

    const SVDOnlineToOfflineMap::ChipInfo& chip_info =
      m_map->getChipInfo(fadc_id, apv_id);
    const bool  is_u     = chip_info.m_uSide;
    const short channel0 = chip_info.m_channel0;
    const short parallel = (chip_info.m_parallel) ? 1 : -1;

    //short pedestal [128] = {0};
    short threshold[128] = {0};
    bool  is_good  [128];
    for (int channel_i = 0; channel_i < 128; channel_i++) {
      const unsigned short strip =
        channel0 + ((unsigned short)channel_i) * parallel;

      //pedestal [channel_i] = (int)(m_noiseMap->getPedestal (vxd_id, is_u,  strip));
      threshold[channel_i] = (int)(m_noiseMap->getThreshold(vxd_id, is_u,  strip));
      is_good  [channel_i] =       m_noiseMap->isGood(vxd_id, is_u,  strip);

      if (!is_good[channel_i]) continue;

      const short* corradc_ptr = (m_enableFineCMC) ? tp_digit->getFineCorrADC(channel_i) : tp_digit->getCorrADC(channel_i);
      bool findHit = false;
      for (int sample_i = 0; sample_i < nSample; sample_i++) {
        B2INFO("corrADC: " << corradc_ptr[sample_i] << ", threshold : " << threshold[channel_i]);
        if (corradc_ptr[sample_i] > threshold[channel_i]) {
          B2INFO("find hit!")
          findHit = true;
          break;
        }
      }// for(int sample_i=0;sample_i<nSample;sample++) {

      if (findHit) {
        B2INFO("tp_digit index: " << index);
        B2INFO(tp_digit->print());
        for (int sample_i = 0; sample_i < nSample; sample_i++) {
          SVDDigit* newDigit = new SVDDigit(vxd_id, is_u, strip, 0.0, ((corradc_ptr[sample_i] > 0) ? corradc_ptr[sample_i] : 0), sample_i, index);
          newDigit->setTime(-m_APVLatency + sample_i * m_APVSamplingTime);
          short current_id = svdDigits->getEntries();
          if (sample_i == 0) {
            newDigit->setPrevID(-1);
            newDigit->setNextID(current_id + 1);
          } else if (sample_i == nSample - 1) {
            newDigit->setPrevID(current_id - 1);
            newDigit->setNextID(-1);
          } else {
            newDigit->setPrevID(current_id - 1);
            newDigit->setNextID(current_id + 1);
          }

          //B2DEBUG(3,newDigit->print());
          //B2INFO(newDigit->print());
          B2INFO("Parent: " << newDigit->getParentTpIndex());
          svdDigits->appendNew(*newDigit);
          m_NewDigit++;
          delete newDigit;
        }// for(int sample_i=0;sample_i<nSample;sample++) {
      }

    }// for(int channel_i=0;channel_i<128;channel_i++) {

  }// for(int index=0;index<nTpDigits;index++) {

  return;
}

void SVDUnpackerModule::printDebug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords)
{

  uint32_t* min = std::max((data32 - nWords), data32_min);
  uint32_t* max = std::min((data32 + nWords), data32_max);

  uint32_t* ptr = min;
  int counter = 0;

  while (ptr < max + 1) {

    printf("%.8x ", *ptr);
    if (counter++ % 10 == 9) printf("\n");

    ptr++;
  }

  printf("\n");
  printf("\n");
  return;

}

void SVDUnpackerModule::printB2Debug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords)
{

  uint32_t* min = std::max((data32 - nWords), data32_min);
  uint32_t* max = std::min((data32 + nWords), data32_max);

  uint32_t* ptr = min;
  int counter = 0;

  char message[256] = "";
  ostringstream os;
  os << endl;

  while (ptr < max + 1) {

    sprintf(message, "%s%.8x ", message, *ptr);
    if (counter++ % 10 == 9) {
      os << message << endl;
      //sprintf(message,"");
      strcpy(message, "");
    }

    ptr++;
  }

  os << message << endl;
  B2DEBUG(1, os.str());

  return;

}
