/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DummyDataPacker.cc
// Description : Module to store dummy data in RawSVD(and others) object
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 14 - Jul - 2014
//-


// The format of readable hex dump
// ************************************************
// Program to read events by DMA
// Data mode
// SUCCESS: Device opened for ECS 0
// SUCCESS: Device opened for ECS 2
// SUCCESS: Device opened for DMA
// Header  : eeee000c aaaaeeee 0180aaaa 00000000 00010000 00000000 00000000 00000000
//  data   0 : eeee000c aaaaeeee 0180aaaa 00000000 00010000 00000000 00000000 00000000
//  data   7 : 0000004e 7f7f0438 0343cd00 00000000 21ed8e47 5f7e6a02 00000000 00000001
//  data  15 : 00000038 00000038 00000038 00000038 00000038 00000038 00000038 00000038
//  data  23 : 00000038 00000038 00000038 00000038 0000004a 0000004a 0000004a 0000004a
//  data  31 : 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a
//  data  39 : 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a
//  data  47 : 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a
//  data  55 : 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a 0000004a
//  data  63 : ffaa0b00 21ed8e47 00000000 5f7e6a02 0343cd00 21ed9c30 4898121d 06a40000
//  data  71 : 4cb01218 06840000 540110de 04980000 5401120f 060c0000 12ee0000 21ed8e47
//  data  79 : 00006408 ff550000 7fff1775 7fff0006 7fff0007 7fff0008 7fff0009 7fff000a
//  data  87 : Trailer :        0       0       0       0       0       0       0       C
// Bad event number prev 0 cur 0
// Header  : eeee000c aaaaeeee 0180aaaa 00000000 00010000 00000001 00000000 00000000
//  data   0 : eeee000c aaaaeeee 0180aaaa 00000000 00010000 00000001 00000000 00000000
//  data   7 : 0000004c 7f7f0438 0343cd00 00000001 21fc9f57 5f7e6a02 00000000 00000001
// ************************************************
#include <iostream>
#include <rawdata/modules/HexDataPacker.h>

using namespace std;
using namespace Belle2;

#define USE_PCIE40

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(HexDataPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

HexDataPackerModule::HexDataPackerModule() : Module()
{
  ///Set module properties
  setDescription("an Example to pack data to a RawCOPPER object");
  addParam("inputFileName", m_fileName, "Output binary filename", string(""));
  B2INFO("HexDataPacker: Constructor done.");
  // initialize event #
  n_basf2evt = 0;
}



HexDataPackerModule::~HexDataPackerModule()
{

}


void HexDataPackerModule::initialize()
{
  B2INFO("HexDataPacker: initialize() started.");

  // Open message handler
  //  m_msghandler = new MsgHandler(m_compressionLevel);

  /// Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  m_raw_cprarray.registerInDataStore();
  m_raw_svdarray.registerInDataStore();
  m_raw_cdcarray.registerInDataStore();
  m_raw_bpidarray.registerInDataStore();
  m_raw_epidarray.registerInDataStore();
  m_raw_eclarray.registerInDataStore();
  m_raw_klmarray.registerInDataStore();

  B2INFO("HexDataPacker: initialize() done.");

  if (m_fileName == "") {
    B2FATAL("HexDataPacker can't fine a filename: ");
  }

  m_ifs.open(m_fileName.c_str());
  if (!m_ifs) {
    B2FATAL("HexDataPacker can't open a file " << m_fileName.c_str());
  }
}




void HexDataPackerModule::event()
{
  StoreArray<RawKLM> ary; // You need to change if you want to make a different Raw*** object.
  const int MAX_CPRBUF_WORDS = 5000;
  int* evtbuf = new int[MAX_CPRBUF_WORDS];
  char char1[50], char2[50], char3[50], char4[50];

  unsigned int val[10];
  int size = 0;
  int runEnd = 0;
  int word_count = 0;
  int event_end = 0;
  while (true) {

    if (m_ifs.eof()) {
      runEnd = 1;
      break;
    }
    string strin;
    getline(m_ifs, strin);
    sscanf(strin.c_str(), "%49s %49s %49s %49s",
           char1, char2, char3, char4);
    if (strcmp(char1, "data") == 0) {
      if (strcmp(char4, "Trailer") == 0) {
        continue;
      }
      if (strcmp(char2, "0") == 0) {
        continue;
      } else if (strcmp(char2, "7") == 0) {
        sscanf(strin.c_str(), "%49s %u %49s %x %x %x %x %x %x %x %x",
               char1, &(val[0]), char2, &(val[1]), &(val[2]), &(val[3]), &(val[4]), &(val[5]), &(val[6]), &(val[7]), &(val[8]));
        size = val[1];
        val[2] = val[2] | 0x00008000; // For data which was not reduced in FPGA
        if (size < 0 || size > MAX_CPRBUF_WORDS) {
          B2FATAL("The size of an event =(" << size << ") is too large. Exiting...");
        }
      } else {
        sscanf(strin.c_str(), "%20s %u %20s %x %x %x %x %x %x %x %x",
               char1, &(val[0]), char2, &(val[1]), &(val[2]), &(val[3]), &(val[4]), &(val[5]), &(val[6]), &(val[7]), &(val[8]));
      }
      if (size <= 0) {
        B2FATAL("The size of an event (=" << size << ") is too large. Exiting...");
      } else {
        for (int i = 1; i <= 8 ; i++) {
          if (word_count >= MAX_CPRBUF_WORDS) {
            B2FATAL("The size of an event (=" << size << ") is too large. Exiting...");
          }
          evtbuf[word_count] = val[i];
          word_count++;
          if (word_count == size) {
            event_end = 1;
            break;
          }
        }
        if (event_end == 1) {break;}
      }
    }
  }

#ifdef DEBUG
  if (size > 0 && size < MAX_CPRBUF_WORDS) {
    for (int i = 0; i < size; i++) {
      printf("%.8x ", evtbuf[i]);
      if (i % 8 == 7) printf("\n");
    }
    printf("\n");
  }
#endif
  StoreObjPtr<EventMetaData> evtmetadata;
  if (runEnd == 0) {
    (ary.appendNew())->SetBuffer(evtbuf, size, 1, 1, 1);

    //
    // Update EventMetaData :
    //
    RawCOPPER tempcpr;
    tempcpr.SetBuffer(evtbuf, size, false, 1, 1);


    evtmetadata.create();
    evtmetadata->setExperiment(tempcpr.GetExpNo(0));
    evtmetadata->setRun(tempcpr.GetRunNo(0));
    evtmetadata->setSubrun(tempcpr.GetSubRunNo(0));
    evtmetadata->setEvent(tempcpr.GetEveNo(0));
    //  evtmetadata->setTime(mtime);  //time(NULL));

  } else {
    delete[] evtbuf;
    evtmetadata.create();
    evtmetadata->setEndOfData();
  }

  printf("Event counter %8d\n", n_basf2evt); fflush(stdout);
  n_basf2evt++;

}
