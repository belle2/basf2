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

}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{

  //commented out ssince daq/rawdata/RawSVD is not in the release:
  //  StoreArray<RawSVD>::required(m_rawSVDListName);
  StoreArray<SVDDigit>::registerPersistent(m_svdDigitListName);

  loadMap();

}

void SVDUnpackerModule::beginRun()
{
}

void SVDUnpackerModule::event()
{
  //commented out ssince daq/rawdata/RawSVD is not in the release:
  //  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  svdDigits.create();

  //commented out ssince daq/rawdata/RawSVD is not in the release
  /*  for (int i = 0; i < rawSVDList.getEntries(); i++) {
    for (int j = 0; j < rawSVDList[ i ]->GetNumEntries(); j++) {

      int nWords = rawSVDList[i]->Get1stDetectorNwords(j); // * sizeof(int) bytes
      uint32_t* data32 = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j);

      //first check the payload checksum:
      checksum();

      fillSVDDigitList(data32, &svdDigits);

    }
  }
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


  //read APV Header:
  struct APVHeader* theAPVHeader = (struct APVHeader*)(data32++);

  if (theAPVHeader->check != 0 + 2)
    B2WARNING("OOOOPS: WRONG APV header format");
  B2DEBUG(1, "APV header format checked");

  //read data samples (if there):

  struct data* aSample;
  struct trailer* theTrailer;

  double charge = 0;

  bool trailerFound = false;

  while (!trailerFound) {
    data32++;

    if (((*data32 >> 31) & 0) == 0) {    //data type

      aSample = (struct data*) data32;

      if (aSample->check != 0)  //should always be OK
        B2WARNING("OOOOPS: WRONG Data Sample format");
      B2DEBUG(1, "Data Sample format checked");

      if ((aSample->sample2 == 0) && (aSample->sample1 == 0) && (aSample->sample0 == 0))
        B2DEBUG(1, "no Data Recorded");
      B2DEBUG(1, "at least a hit is present");

      svdDigits->appendNew(*(m_map->NewDigit(theMainHeader->FADCnum, theAPVHeader->APVnum, aSample->stripNum)));

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
