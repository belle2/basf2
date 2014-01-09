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

#include <iomanip>

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
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  addParam("svdDigitListName", m_svdDigitListName, "Name of the SVD Digits List", string(""));
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", string(""));
  addParam("APVLatency", m_APVLatency, " APV latency (in ns)", float(0));
  addParam("APVSamplingTime", m_APVSamplingTime, " APV sampling time (in ns)", float(1));

}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{

  StoreArray<RawSVD>::required(m_rawSVDListName);
  StoreArray<SVDDigit>::registerPersistent(m_svdDigitListName);

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
  m_wrongFTBtrailer = 0;

  m_noAPVHeader = 0;

}

void SVDUnpackerModule::event()
{
  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  svdDigits.create();

  if (! m_map) {
    B2ERROR("xml map not loaded, going to the next module");
    return;
  }

  for (int i = 0; i < rawSVDList.getEntries(); i++) {
    for (int j = 0; j < rawSVDList[ i ]->GetNumEntries(); j++) {

      int nWords = rawSVDList[i]->Get1stDetectorNwords(j);
      uint32_t* data32 = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j);

      if (sanityChecks(nWords, data32))
        fillSVDDigitList(nWords, data32, &svdDigits);
      else
        m_failedChecks++;
    }
  }


  //  B2INFO("number of entries of the RawSVD list = "<< rawSVDList.getEntries());
  //  for (int i = 0; i < rawSVDList.getEntries(); i++)
  //    B2INFO(" number of entries of the current RawSVD object = "<< rawSVDList[ i ]->GetNumEntries())

}

void SVDUnpackerModule::endRun()
{

  B2INFO(" total failed checks = " << m_failedChecks);
  B2INFO("   m_wrongFTBHeader = " <<  m_wrongFTBHeader);
  B2INFO("   m_wrongFADCTrailer = " << m_wrongFADCTrailer);
  B2INFO("   m_wrongFADCcrc = " << m_wrongFADCcrc);
  B2INFO("   m_wrongFTBtrailer = " << m_wrongFTBtrailer);


  B2INFO(" m_noAPVHeader = " <<  m_noAPVHeader);

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
}

void SVDUnpackerModule::loadMap()
{

  //load the sensor MAP from xml file
  m_map = new SVDOnlineToOfflineMap(m_xmlMapFileName);

}

bool SVDUnpackerModule::sanityChecks(int nWords, uint32_t* data32)
{

  //first of all verify FTB checksum, then proceed with FTB/FADC headers and trailers

  //read FTB Trailer
  struct FTBTrailer* theFTBTrailer = (struct FTBTrailer*) &data32[nWords - 1];
  if (theFTBTrailer->controlWord != 0xff55) {
    B2ERROR("WRONG FTB Trailer");
    m_wrongFTBtrailer++;
    return false;
  } else
    B2DEBUG(1, "sanityChecks: FTB Trailer found");
  // verify FTB checksum
  if (! verifyFTBcrc())
    return false;


  //read FTB Header:
  struct FTBHeader* theFTBHeader = (struct FTBHeader*)data32;
  if (theFTBHeader->controlWord != 0xffaa0000) {
    B2ERROR("WRONG FTB header format 0x" << std::hex << std::setw(8) << std::setfill('0') << theFTBHeader->controlWord);
    m_wrongFTBHeader++;
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
  struct MainHeader* theMainHeader = (struct MainHeader*) &data32[2];
  if (theMainHeader->check != 0x6) {
    B2ERROR("WRONG FADC main header format 0x" << std::hex << std::setw(8) << std::setfill('0') << theMainHeader->check);
    m_wrongFADCHeader++;
    return false;
  } else
    B2DEBUG(1, "main header format checked");
  //check the run type
  if (theMainHeader->runType != 0x2) {
    B2ERROR("WRONG run type (expected = zero-suppressed), got 0x" << std::hex << std::setw(8) << std::setfill('0') << theMainHeader->runType);
    m_wrongRunType++;
    return false;
  } else
    B2DEBUG(1, "run type checked (zerosuppressed)");

  //read FADC Trailer
  struct FADCTrailer* theFADCTrailer = (struct FADCTrailer*) &data32[nWords - 2];
  if (theFADCTrailer->check != 0xe) {
    B2ERROR("WRONG FADC Trailer");
    m_wrongFADCTrailer++;
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
    }

  return true;

}

bool SVDUnpackerModule::verifyFTBcrc()
{
  //verify the checksum - to be implemented
  return true;
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
  float time = m_APVLatency;

  bool FADCtrailerFound = false;

  for (; !FADCtrailerFound  && (data32 != &data32_in[nWords]); ++data32) {

    if (((*data32 >> 30) & 0x3) == 0x2) { //APV header type

      theAPVHeader = (struct APVHeader*) data32;
      B2DEBUG(1, "New APV header");

      continue;
    }

    if (((*data32 >> 31) & 0x1) == 0) {     //zero-suppressed data

      aSample = (struct data*) data32;
      previousSample = (struct data*)(data32 - 1);

      if (aSample->stripNum != previousSample->stripNum)
        time = m_APVLatency;

      //add the 3 data samples:
      if (theAPVHeader) {
        for (int i = 0; i < 3; i++) {
          SVDDigit* newDigit = m_map->NewDigit(theMainHeader->FADCnum, theAPVHeader->APVnum, aSample->stripNum, aSample->sample[i], time);
          time += m_APVSamplingTime;

          // Translation can return 0, if wrong FADC/APV combination is encountered.
          if (!newDigit) {
            B2WARNING("Unknown FADC #" << theMainHeader->FADCnum << " and APV #" << theAPVHeader->APVnum);
            continue;
          } else {
            svdDigits->appendNew(*newDigit);
            delete newDigit;
          }
        }
      } else
        m_noAPVHeader++;
      //  B2WARNING(" FADC data before a valid APV header 0x" << std::hex << std::setw(8) << std::setfill('0') << *data32);


      continue;
    }

    if (((*data32 >> 28) & 0xf) == 0xe) { // FADC trailer type

      B2DEBUG(1, "FADC Trailer found");
      FADCtrailerFound = true;

      continue;
    }

    B2WARNING("unknown data field, highest four bits: 0x" << std::hex << std::setw(8) << std::setfill('0') << *data32 << " data check bit is = " << (*data32 >> 31));
    return;
  }

  if (&data32_in[nWords] != &data32[1]) {
    B2WARNING("FADC trailer appeared too early, data short by " << &data32_in[nWords] - &data32[1] << " bytes");
  }


}
