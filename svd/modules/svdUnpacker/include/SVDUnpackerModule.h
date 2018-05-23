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
#include <svd/dataobjects/SVDDAQDiagnostic.h>

#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/online/SVDStripNoiseMap.h>
#include <framework/dataobjects/EventMetaData.h>
#include <memory>
#include <framework/database/DBObjPtr.h>
#include <framework/database/PayloadFile.h>

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
      virtual void initialize() override;
      virtual void beginRun() override;
      virtual void event() override;
      virtual void endRun() override;

      std::string m_rawSVDListName;
      std::string m_svdDigitListName;

      bool m_generateOldDigits;
      std::string m_svdShaperDigitListName;
      std::string m_svdDAQDiagnosticsListName;

      int m_wrongFTBcrc;



    private:

      /**how many FADCs we have */
      unsigned short nFADCboards;

      /** pointer to APVforFADCmap filled by mapping procedure */
      std::unordered_multimap<unsigned char, unsigned char>* APVmap;

      std::unique_ptr<SVDOnlineToOfflineMap> m_map;
      //unsigned short m_runType;
      static std::string m_xmlFileName;
      DBObjPtr<PayloadFile> m_mapping;

      SVDModeByte m_SVDModeByte;

      void printB2Debug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords);

      // The following assumes i386 byte order: MSB comes last!

      struct FTBHeader {
        // unsigned int controlWord : 32; //LSB
        unsigned int errorsField : 8; //LSB
        unsigned int eventNumber : 24; //MSB
      };


      struct MainHeader {
        unsigned int trgNumber : 8; //LSB
        unsigned int trgType   : 4;
        unsigned int trgTiming : 3;
        unsigned int onebit    : 1;
        unsigned int FADCnum   : 8;
        unsigned int evtType   : 1; // Event type(0): 0…TTD event, 1…standalone event
        unsigned int DAQMode   : 2; // Event type(2:1): "00"…1-sample, "01"…3-sample, "10"…6-sample
        unsigned int runType   : 2;
        unsigned int check     : 3; //MSB
      };

      struct APVHeader {
        unsigned int CMC1      : 8; //LSB
        unsigned int CMC2      : 4;
//         unsigned int fifoErr   : 1;
//         unsigned int frameErr  : 1;
//         unsigned int detectErr : 1;
        unsigned int apvErr    : 4;
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
        unsigned int emuPipeAddr: 8;
//         unsigned int fifoErrOR   : 1;
//         unsigned int frameErrOR  : 1;
//         unsigned int detectErrOR : 1;
        unsigned int apvErrOR  : 4;
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

      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;   /**< Required input for EventMetaData */
      StoreArray<RawSVD> m_rawSVD;   /**< output for RawSVD */
      StoreArray<SVDDigit> m_svdDigit; /**< Required input for SVDDigit */

      int m_shutUpFTBError;
      int m_FADCTriggerNumberOffset;

      /** Software emulation of pipeline address
       * This is a replacement of hardware pipeline address emulation.
       * Pipeline address is emulated by major vote from working APVs.
       * Can be turned off once hardware emulation works.
       */
      bool m_emulatePipelineAddress = false;

      /** Optionally, we can kill digits coming from upset APVs
       * right in the unpacker.
       */
      bool m_killUpsetDigits = false;

      /** Silently append new SVDDigits to a pre-existing non-empty
       * SVDDigits/SVDShaperDigits storeArray.
       * If false, a pre-exsiting non-empty output StoreArray will cause
       * a FATAL error to remind the users that they may be mixing data
       * inadvertently, and that they need to plug in a digit sorter in
       * the module path.
       */
      bool m_silentAppend = false;

      /** Optionally we can stop the unpacking if there is a missing
       *  APV/FADC combination in the mapping -> wrong payload is identified
       */
      bool m_badMappingFatal = false;

      /** Map to store a list of missing APVs */
      std::map<std::pair<unsigned short, unsigned short>, std::pair<std::size_t, std::size_t> > m_missingAPVs;

      /** Map to store a list of upset APVs */
      std::map<std::pair<unsigned short, unsigned short>, std::pair<std::size_t, std::size_t> > m_upsetAPVs;


    };//end class declaration


  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDUnpackerModule_H

