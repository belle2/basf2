/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jarek Wiechczynski, Giulia Casarosa, Eugenio Paoloni     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <svd/modules/svdUnpacker/SVDUnpackerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

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
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  addParam("svdDigitListName", m_svdDigitListName, "Name of the SVD Digits List", string(""));
  //addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", string(""));
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", FileSystem::findFile("data/svd/svd_mapping.xml"));

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
  m_wrongFTBcrc = 0;

}

void SVDUnpackerModule::event()
{

  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);


  svdDigits.clear();


  if (! m_map) {
    B2ERROR("xml map not loaded, going to the next module");
    return;
  }

  unsigned int nEntries_rawSVD = rawSVDList.getEntries();
  for (unsigned int i = 0; i < nEntries_rawSVD; i++) {

    unsigned int numEntries_rawSVD = rawSVDList[ i ]->GetNumEntries();
    for (unsigned int j = 0; j < numEntries_rawSVD; j++) {

      unsigned short nWords[4];
      nWords[0] = rawSVDList[i]->Get1stDetectorNwords(j);
      nWords[1] = rawSVDList[i]->Get2ndDetectorNwords(j);
      nWords[2] = rawSVDList[i]->Get3rdDetectorNwords(j);
      nWords[3] = rawSVDList[i]->Get4thDetectorNwords(j);

      // i,j is only 0

      uint32_t* data32tab[4]; //vector of pointers

      data32tab[0] = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j); // points at the begining of the 1st buffer
      data32tab[1] = (uint32_t*)rawSVDList[i]->Get2ndDetectorBuffer(j);
      data32tab[2] = (uint32_t*)rawSVDList[i]->Get3rdDetectorBuffer(j);
      data32tab[3] = (uint32_t*)rawSVDList[i]->Get4thDetectorBuffer(j);

      for (unsigned int buf = 0; buf < 4; buf++) { // loop over 4 buffers

        //printB2Debug(data32tab[buf], data32tab[buf], &data32tab[buf][nWords[buf] - 1], nWords[buf]);

        uint32_t* data32_it = data32tab[buf];
        short fadc = 255, apv = 63, strip, sample[6];
        vector<uint32_t> crc16vec;

        for (; data32_it != &data32tab[buf][nWords[buf]]; data32_it++) {
          data32 = *data32_it; //pu current 32-bit frame to union


          if (data32 == 0xffaa0000) {   // first part of FTB header
            crc16vec.clear(); // clear the input container for crc16 calculation
            crc16vec.push_back(data32);
            data32_it++; // go to 2nd part of FTB header
            crc16vec.push_back(*data32_it);
            continue;
          }

          crc16vec.push_back(data32);


          if (m_MainHeader.check == 6)  // FADC header
            fadc = m_MainHeader.FADCnum;


          if (m_APVHeader.check == 2)  // APV header
            apv = m_APVHeader.APVnum;


          if (m_data_A.check == 0) { // data
            strip = m_data_A.stripNum;

            sample[0] = m_data_A.sample1;
            sample[1] = m_data_A.sample2;
            sample[2] = m_data_A.sample3;

            data32_it++;
            data32 = *data32_it; // 2nd frame with data
            crc16vec.push_back(data32);

            sample[3] = m_data_B.sample4;
            sample[4] = m_data_B.sample5;
            sample[5] = m_data_B.sample6;


            for (unsigned int i = 0; i < 6; i++) {
              // m_cellPosition member of the SVDDigit object is set to zero by NewDigit function
              SVDDigit* newDigit = m_map->NewDigit(fadc, apv, strip, sample[i], i);
              svdDigits.appendNew(*newDigit);

              delete newDigit;
            }


          }  //is data frame

          if (m_FTBTrailer.controlWord == 0xff55)  {// FTB trailer

            //check CRC16
            crc16vec.pop_back();
            unsigned short iCRC = crc16vec.size();
            //uint32_t *crc16input = new uint32_t[iCRC];
            uint32_t crc16input[iCRC];

            for (unsigned short i = 0; i < iCRC; i++)
              crc16input[i] = htonl(crc16vec.at(i));

            //verify CRC16
            boost::crc_basic<16> bcrc(0x8005, 0xffff, 0, false, false);
            bcrc.process_block(crc16input, crc16input + iCRC);
            unsigned int checkCRC = bcrc.checksum();

            if (checkCRC != m_FTBTrailer.crc16) {
              B2WARNING("FTB CRC16 checksum DOES NOT MATCH for FADC no. " << fadc);
              m_wrongFTBcrc++;
            }

          } // FTB trailer
        } // end loop over 32-bit frames in each buffer

      } // end iteration on 4 data buffers

      //m_runType = 0;

    } // end event loop

  }
} //end event function

void SVDUnpackerModule::endRun()
{
  B2INFO("   m_wrongFTBcrc = " << m_wrongFTBcrc);
}


void SVDUnpackerModule::terminate()
{
  delete m_map;

}

//load the sensor MAP from xml file
void SVDUnpackerModule::loadMap()
{
  m_map = new SVDOnlineToOfflineMap(m_xmlMapFileName);

}


// additional printing function
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
    char prev_message[256] = "";
    strcpy(prev_message, message);
    sprintf(message, "%s%.8x ", prev_message, *ptr);
    if (counter++ % 10 == 9) {
      os << message << endl;
      //sprintf(message,"");
      strcpy(message, "");
    }

    ptr++;
  }

  os << message << endl;
  //B2DEBUG(1, os.str());
  B2INFO(os.str());
  return;

}
