/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski,  Giulia Casarosa, Eugenio Paoloni    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDUnpackerModule_H
#define SVDUnpackerModule_H

#include <framework/core/Module.h>

#include <svd/geometry/SensorInfo.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTransparentDigit.h>

#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/online/SVDStripNoiseMap.h>
#include <framework/dataobjects/EventMetaData.h>

namespace Belle2 {

  namespace SVD {

    /** SVDUnpackerModule: The SVD Raw Hits Decoder.
     *
     * This module produces SVDDigits from the Copper
     */
    class SVDUnpackerModule : public Module {

    public:
      /**
       * Constructor of the module.
       */
      SVDUnpackerModule();

      /**
       * Destructor of the module.
       */
      virtual ~SVDUnpackerModule();

      /**
       *Initializes the Module.
       */
      virtual void initialize();
      virtual void beginRun();
      virtual void event();
      virtual void endRun();

      virtual void terminate();

      std::string m_rawSVDListName;
      std::string m_svdDigitListName;
      std::string m_xmlMapFileName;

      bool m_generateShaperDigts;
      std::string m_svdShaperDigitListName;

      int m_wrongFTBcrc;


    private:

      SVDOnlineToOfflineMap* m_map;
      //unsigned short m_runType;

      void loadMap();
      void printB2Debug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords);

      // The following assumes i386 byte order: MSB comes last!

      struct FTBHeader {
        // unsigned int controlWord : 32; //LSB
        unsigned int errorsField : 8; //LSB
        unsigned int eventNumber : 24; //MSB
      };


      struct MainHeader {
        unsigned int trgNumber : 8; //LSB
        unsigned int trgTiming : 8;
        unsigned int FADCnum   : 8;
        unsigned int evtType   : 3;
        unsigned int runType   : 2;
        unsigned int check     : 3; //MSB
      };

      struct APVHeader {
        unsigned int CMC1      : 8; //LSB

        unsigned int CMC2      : 4;
        unsigned int errorMask : 3;
        unsigned int errorBit  : 1;

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

        unsigned int wiredOrErr: 1;
        unsigned int error0: 1;
        unsigned int error1: 1;
        unsigned int error2: 1;
        unsigned int check : 4; //MSB
      };

      struct FTBTrailer {
        unsigned int crc16 : 16; //LSB
        unsigned int controlWord : 16; //MSB
      };



      union {  // The 4 byte words of the stream can be interpreted as:
        uint32_t m_data32;
        FTBHeader m_FTBHeader;
        MainHeader m_MainHeader;
        APVHeader m_APVHeader;
        data_A  m_data_A;
        data_B  m_data_B;
        FADCTrailer m_FADCTrailer;
        FTBTrailer m_FTBTrailer;
      };

      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

      int m_shutUpFTBError;
      int m_FADCTriggerNumberOffset;

    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDUnpackerModule_H

