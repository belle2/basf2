/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDPackerModule_H
#define SVDPackerModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VxdID.h>

#include <rawdata/dataobjects/RawSVD.h>
#include <svd/dataobjects/SVDDigit.h>

#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/online/SVDStripNoiseMap.h>

#include <framework/database/DBObjPtr.h>
#include <framework/database/PayloadFile.h>
//


namespace Belle2 {
  namespace SVD {


    class SVDPackerModule : public Module {

    public:

      //Constructor

      SVDPackerModule();

      virtual ~SVDPackerModule();

      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();

      virtual void terminate();


      std::string m_rawSVDListName;
      std::string m_svdDigitListName;


      bool m_simulate3sampleData;


    private:
      //   table of FADC numbers as in xml file      forward  |  backward
      const unsigned short iFADCnumber[48] = {1, 129,     33, 161,
                                              8, 9, 136, 137,     40, 41, 42, 168, 169, 170,
                                              16, 17, 144, 145,     48, 49, 50, 51, 52, 176, 177, 178, 179, 180,
                                              24, 25, 26, 27, 152, 153, 154, 155,     56, 57, 58, 59, 60, 61, 184, 185, 186, 187, 188, 189
                                             };

      int n_basf2evt; //event number
      int m_nodeid; // Node ID

      static std::string m_xmlFileName;
      DBObjPtr<PayloadFile> m_mapping;

      std::unique_ptr<SVDOnlineToOfflineMap> m_map;
      //SVDStripNoiseMap* m_noiseMap;
      std::unordered_map<unsigned short, unsigned short> fadcNumbers;
      short iCRC;
      std::vector<uint32_t> data_words;
      uint32_t crc16Input[1000];

      //adds data32 to data vector and to crc16Input for further crc16 calculation
      void inline addData32(uint32_t data32)
      {
        data_words.push_back(data32);
        crc16Input[iCRC] = data32;
        iCRC++;
      }


      void prepFADClist();
      void binPrintout(unsigned int nwords);

      struct DataInfo {
        short data[6];
        unsigned short channel;
      } dataInfo;


      // bit structures
      struct FTBHeader {
        unsigned int errorsField : 8;//LSB
        unsigned int eventNumber : 24; //MSB
      };

      struct MainHeader {
        unsigned int trgNumber : 8; //LSB
        unsigned int trgType : 4;
        unsigned int trgTiming : 3;
        unsigned int onebit : 1;
        unsigned int FADCnum : 8;
        unsigned int evtType : 1; // Event type(0): 0…TTD event, 1…standalone event
        unsigned int DAQMode : 2; // Event type(2:1): "00"…1-sample, "01"…3-sample, "10"…6-sample
        unsigned int runType : 2;
        unsigned int check : 3; //MSB
      };


      struct APVHeader {
        unsigned int CMC1 : 8; //LSB
        unsigned int CMC2 : 4;
        unsigned int fifoErr: 1;
        unsigned int frameErr: 1;
        unsigned int detectErr: 1;
        unsigned int apvErr : 1;
        unsigned int pipelineAddr : 8;
        unsigned int APVnum : 6;
        unsigned int check : 2; //MSB
      };

      struct data_A {
        unsigned int sample1 : 8; //LSB
        unsigned int sample2 : 8;
        unsigned int sample3 : 8;
        unsigned int stripNum : 7;
        unsigned int check : 1; //MSB
      };

      struct data_B {
        unsigned int sample4 : 8; //LSB
        unsigned int sample5 : 8;
        unsigned int sample6 : 8;
        unsigned int stripNum : 7;
        unsigned int check : 1; //MSB
      };


      struct FADCTrailer {
        unsigned int FTBFlags: 16; //LSB
        unsigned int emPipeAddr: 8;
        unsigned int fifoErrOR: 1;
        unsigned int frameErrOR: 1;
        unsigned int detectErrOR: 1;
        unsigned int apvErrOR: 1;
        unsigned int check : 4; //MSB
      };


      struct FTBTrailer {
        unsigned int crc16 : 16; //LSB
        unsigned int controlWord : 16; //MSB
      };


      union {
        uint32_t data32; // output
        FTBHeader m_FTBHeader;
        MainHeader m_MainHeader;
        APVHeader m_APVHeader;
        data_A  m_data_A;
        data_B  m_data_B;
        FADCTrailer m_FADCTrailer;
        FTBTrailer m_FTBTrailer;
      };

      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
      int m_FADCTriggerNumberOffset;

    };
  } //SVD
} // Belle2

#endif /* SVDPackerModule_H */
