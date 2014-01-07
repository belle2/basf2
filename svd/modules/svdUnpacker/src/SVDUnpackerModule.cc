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

//temporary, to read Katsuro's files from Vienna test
// /*
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
// */

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

  //temporary, to read Katsuro's flies from Vienna test:
  addParam("dataFileName", tmp_dataFileName, "TMP: dat filename", string(""));

}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{

  //RawSVD.h disappeared from the release (reappeared):
  StoreArray<RawSVD>::required(m_rawSVDListName);
  StoreArray<SVDDigit>::registerPersistent(m_svdDigitListName);

  //commented out since no map exists at the moment
  loadMap();
}

void SVDUnpackerModule::beginRun()
{
}

void SVDUnpackerModule::event()
{
  //RawSVD.h disappeared from the release (reappeared):
  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  svdDigits.create();

  for (int i = 0; i < rawSVDList.getEntries(); i++) {
    for (int j = 0; j < rawSVDList[ i ]->GetNumEntries(); j++) {

      //to be used to check the length:
      int nWords = rawSVDList[i]->Get1stDetectorNwords(j); // * sizeof(int) bytes
      uint32_t* data32 = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j);

      //first check the payload checksum:
      checksum();

      // Skip two words that I don't understand.
      //      data32 += 2;
      //      fillSVDDigitList(nWords - 2, data32, &svdDigits);

      fillSVDDigitList(nWords, data32, &svdDigits);

    }
  }
}

void SVDUnpackerModule::endRun()
{
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

void SVDUnpackerModule::checksum()
{
  //check the checksum
}


void SVDUnpackerModule::fillSVDDigitList(int nWords, uint32_t* data32_in,  StoreArray<SVDDigit>* svdDigits)
{
  uint32_t* data32 = data32_in;

  //read FTB Header:
  struct FTBHeader* theFTBHeader = (struct FTBHeader*)data32;

  if (theFTBHeader->controlWord != 0xffaa0000) {
    B2WARNING("OOOOPS: WRONG FTB header format 0x" << std::hex << theFTBHeader->controlWord);
    return;
  }
  B2DEBUG(1, "FTB header format checked");


  data32 += 2;
  //read Main Header:
  struct MainHeader* theMainHeader = (struct MainHeader*)data32;

  if (theMainHeader->check != 0x6) {
    B2WARNING("OOOOPS: WRONG main header format 0x" << std::hex << theMainHeader->check);
    return;
  }
  B2DEBUG(1, "main header format checked");

  //check run typea
  if (theMainHeader->runType != 0x2) {
    B2WARNING("OOOOPS: WRONG main runType (expected = zero-suppressed), got 0x" << std::hex << theMainHeader->runType);
    return;
  }
  B2DEBUG(1, "run type checked (zerosuppressed)");

  //read APV Header:
  struct APVHeader* theAPVHeader = (struct APVHeader*)(++data32);

  if (theAPVHeader->check != 0x2) {
    B2WARNING("OOOOPS: WRONG APV header format 0x" << std::hex << theAPVHeader->check);
    return;
  }
  B2DEBUG(1, "APV header format checked");

  //read data samples (if there):

  struct data* aSample;
  struct trailer* theTrailer;
  struct FTBTrailer* theFTBTrailer;

  bool trailerFound = false;

  ++data32;
  for (; !trailerFound  && (data32 != &data32_in[nWords]); ++data32) {

    if (((*data32 >> 31) & 0x1) == 0) {    //zero-suppressed data

      aSample = (struct data*) data32;

      assert(aSample->check == 0);

      //add the 3 data samples:
      if (m_map) //remove check?
        for (int i = 0; i < 3; i++) {
          SVDDigit* newDigit = m_map->NewDigit(theMainHeader->FADCnum, theAPVHeader->APVnum, aSample->stripNum, aSample->sample[i], theMainHeader->trgTiming);
          // Translation can return 0, if wrong FADC/APV combination is encountered.
          if (!newDigit) {
            B2WARNING("Unkown FADC #" << theMainHeader->FADCnum << " and APV #" << theAPVHeader->APVnum);
            continue;
          }
          svdDigits->appendNew(*newDigit);
          delete newDigit;
        }


    } else if (((*data32 >> 30) & 0x3) == 0x2) { //APV header type
      theAPVHeader = (struct APVHeader*) data32;
      assert(theAPVHeader->check == 2 + 0);
      B2DEBUG(1, "New APV header");

    } else if (((*data32 >> 28) & 0xf) == 0xe) { // trailer type

      //read the trailer

      theTrailer = (struct trailer*) data32;
      assert(theTrailer->check == 0xe);
      B2DEBUG(1, "Trailer found");

      data32++;
      theFTBTrailer = (struct FTBTrailer*) data32;
      assert(theFTBTrailer->controlWord == 0xff55);
      B2DEBUG(1, "FTBTrailer found");

      trailerFound = true;
    } else {
      B2WARNING("OOOOPS: unknown data field, highest four bits: 0x" << std::hex << *data32);
      return;
    }
  }

  if (&data32_in[nWords] != &data32[1] - 1) {
    B2WARNING("OOOOPS: trailer appeared too early, data short by " << &data32_in[nWords] - &data32[1] << " bytes");
  }

  if (! trailerFound)
    B2ERROR("OOOOPS: read data block without trailer");
}

//temporary: to check format from Vienna test files:
/*
  void SVDUnpackerModule::printbitssimple(int n, int nBits) {
 //Print n as a binary number

  unsigned int i;

  i = 1<<(nBits - 1);

  while (i > 0) {
    if (n & i)
      printf("1");
    else
      printf("0");
    i >>= 1;
  }
}
*/
