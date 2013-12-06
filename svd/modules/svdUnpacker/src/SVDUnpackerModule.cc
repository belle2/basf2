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
/*
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
*/

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
  //  addParam("dataFileName", tmp_dataFileName, "TMP: dat filename", string(""));

}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{

  //RawSVD.h disappeared from the release:
  // StoreArray<RawSVD>::required(m_rawSVDListName);
  StoreArray<SVDDigit>::registerPersistent(m_svdDigitListName);

  //commented out since no map exists at the moment
  //  loadMap();

}

void SVDUnpackerModule::beginRun()
{
}

void SVDUnpackerModule::event()
{

  //RawSVD.h disappeared from the release:
  //  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  svdDigits.create();

  //      //RawSVD.h disappeared from the release:
  /*
    for (int i = 0; i < rawSVDList.getEntries(); i++) {
    for (int j = 0; j < rawSVDList[ i ]->GetNumEntries(); j++) {

      //to be used to check the length:
      //      int nWords = rawSVDList[i]->Get1stDetectorNwords(j); // * sizeof(int) bytes
      uint32_t* data32 = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j);

      //first check the payload checksum:
      checksum();

      fillSVDDigitList(data32, &svdDigits);

      }
  }
  */


  /*
  //temporary: to check format from Vienna test files:
  int file = open(tmp_dataFileName.c_str(), O_RDONLY);
  size_t len = 4000000; //in bytes 4
  uint32_t* data32 = (uint32_t*) mmap(0, len, PROT_READ, MAP_PRIVATE, file, 0);
  printf("PRINT: %x\n", *data32);

  fillSVDDigitList((uint32_t*)data32, &svdDigits);
  */

}

void SVDUnpackerModule::endRun()
{
}


void SVDUnpackerModule::terminate()
{
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


void SVDUnpackerModule::fillSVDDigitList(uint32_t* data32,  StoreArray<SVDDigit>* svdDigits)
{


  //read Main Header:
  struct MainHeader* theMainHeader = (struct MainHeader*) data32;


  if (theMainHeader->check != 0 + 2 + 4)
    B2WARNING("OOOOPS: WRONG main header format");
  B2DEBUG(1, "main header format checked");

  //check run type
  if (theMainHeader->runType != 0 + 2)
    B2WARNING("OOOOPS: WRONG main runType (expected = zero-suppressed)");
  B2DEBUG(1, "run type checked (zerosuppressed)");

  /*
  //temporary: to check format from Vienna test files:
  printf("CHECK (3 bit) = %x ---> ", theMainHeader->check);
  printbitssimple(theMainHeader->check, 3);
  printf("\nRUN TYPE (2 bit)= %x  ---> ", theMainHeader->runType);
  printbitssimple(theMainHeader->runType, 2);
  printf("\nEVT TYPE (3 bit) = %x  ---> ", theMainHeader->evtType);
  printbitssimple(theMainHeader->evtType, 3);
  printf("\nFADC num (8 bit) = %x  ---> ", theMainHeader->FADCnum);
  printbitssimple(theMainHeader->FADCnum, 8);
  printf("\nTRG tim  (8 bit) = %x  ---> ", theMainHeader->trgTiming);
  printbitssimple(theMainHeader->trgTiming, 8);
  printf("\nTRGnumm  (8 bit) = %x  ---> ", theMainHeader->trgNumber);
  printbitssimple(theMainHeader->trgNumber, 8);
  printf("\n\n");
  */

  //read APV Header:
  struct APVHeader* theAPVHeader = (struct APVHeader*)(data32++);

  if (theAPVHeader->check != 0 + 2)
    B2WARNING("OOOOPS: WRONG APV header format");
  B2DEBUG(1, "APV header format checked");

  //read data samples (if there):

  struct data* aSample;
  struct trailer* theTrailer;

  bool trailerFound = false;

  while (!trailerFound) {
    data32++;

    if (((*data32 >> 31) & 0) == 0) {    //data type

      aSample = (struct data*) data32;

      if (aSample->check != 0)  //should always be OK
        B2WARNING("OOOOPS: WRONG Data Sample format");
      B2DEBUG(1, "Data Sample format checked");

      //add the 3 data samples:
      if (m_map) //remove check?
        for (int i = 0; i < 3; i++)
          svdDigits->appendNew(*(m_map->NewDigit(theMainHeader->FADCnum, theAPVHeader->APVnum, aSample->stripNum, aSample->sample[i], theMainHeader->trgTiming)));


    } else if (((*data32 >> 30) & 0) == 0) //APV header type
      theAPVHeader = (struct APVHeader*) data32;

    else { // trailer type

      //read the trailer

      theTrailer = (struct trailer*) data32;

      if (theTrailer->check == 0 + 2) //should always be OK
        B2WARNING("OOOOPS: WRONG trailer format");
      B2DEBUG(1, "APV header format checked");

      trailerFound = true;
    }
  }
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
