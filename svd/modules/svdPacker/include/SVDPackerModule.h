/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef SVDPackerModule_H
#define SVDPackerModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <svd/dataobjects/SVDEventInfo.h>

#include <rawdata/dataobjects/RawSVD.h>
#include <svd/dataobjects/SVDShaperDigit.h>

#include <svd/online/SVDOnlineToOfflineMap.h>

#include <framework/database/PayloadFile.h>


namespace Belle2 {
  namespace SVD {

    /** SVDPackerModule: The SVD Raw Hits Creator.
     *
     * This module produces SVD Raw Data from simulated SVDShaperDigits
     */
    class SVDPackerModule : public Module {

    public:

      //Constructor

      /** default constructor*/
      SVDPackerModule();

      /**default destructor*/
      virtual ~SVDPackerModule();

      virtual void initialize() override; /**<initialize*/
      virtual void beginRun() override; /**<begin run*/
      virtual void event() override; /**<event*/
      virtual void endRun() override; /**<end run*/
      virtual void terminate() override; /**<terminate*/


      std::string m_rawSVDListName; /**<RawSVD StoreArray name*/
      std::string m_svdShaperDigitListName; /**<SVDShaperDigit StoreArray name*/
      std::string m_svdEventInfoName; /**< SVDEventInfo name */
      bool m_simulate3sampleData; /**<if true, simulate 3-sample data taking*/
      bool m_binPrintout;  /**< if true, print data created by the Packer */

    private:

      /** type def for the FADC map*/
      typedef std::unordered_map<unsigned short, unsigned short> FADCmap;

      /**how many FADCs we have */
      unsigned short nFADCboards;

      /** pointer to APVforFADCmap filled by mapping procedure */
      std::unordered_multimap<unsigned char, unsigned char>* APVmap;

      int n_basf2evt; /**<event number*/
      int m_nodeid; /**< Node ID*/

      static std::string m_xmlFileName /**< xml filename*/;
      DBObjPtr<PayloadFile> m_mapping; /**< channel map payload*/

      std::unique_ptr<SVDOnlineToOfflineMap> m_map; /**< Pointer to online-to-offline map */

      /**maps containing assignment (0,1,2,3,4,..,nFADCboards-1) <-> FADC numbers  */
      FADCmap FADCnumberMap;
      /**maps containing assignment (0,1,2,3,4,..,nFADCboards-1) <-> FADC numbers  */
      FADCmap FADCnumberMapRev;

      std::vector<uint32_t> data_words; /**<vector of raw data words*/

      /** adds packed 32-bit data word to the raw data vector */
      void inline addData32(uint32_t adata32)
      {
        data_words.push_back(adata32);
      }
      /** tool: print out N words*/
      void binPrintout(unsigned int nwords);

      /** 6 samples and APV  channel struct*/
      struct DataInfo {
        short data[6]; /**< 6 samples*/
        unsigned short channel; /**< APV channel number*/
      } dataInfo; /**< data info*/

      // The following assumes i386 byte order: MSB comes last!

      /** implementation of FTB Header */
      struct FTBHeader {
        unsigned int errorsField : 8; /**< FTB error fields */
        unsigned int eventNumber : 24; /**< FTB event number */
      };

      /** implementation of FADC Header */
      struct MainHeader {
        unsigned int trgNumber : 8; /**< Trigger Number */
        unsigned int trgType : 4;   /**< Trigger Type */
        unsigned int trgTiming : 2; /**< Trigger Timing */
        unsigned int xTalk : 2;     /**< cross talk tag */
        unsigned int FADCnum : 8;   /**< FADC number */
        unsigned int DAQType : 1; /**< (from 2020c) Event type(0): "0"…3 or …6 acquisition mode, "1"…3-mixed-6 acquisition mode */
        unsigned int DAQMode : 2; /**< Event type(2:1): "00"…1-sample, "01"…3-sample, "10"…6-sample */
        unsigned int runType : 2; /**< Run Type */
        unsigned int check : 3; /**< MSB "110" - for FADC Header identification */
      };

      /** implementation of APV Header */
      struct APVHeader {
        unsigned int CMC1 : 8; /**< Common Mode Noise w/o masking out particle signals */
        unsigned int CMC2 : 4; /**< Common Mode Noise after masking out particle signals */
        unsigned int fifoErr: 1; /**< FIFO full Error */
        unsigned int frameErr: 1; /**< Frame Error */
        unsigned int detectErr: 1; /**< Detection Error */
        unsigned int apvErr : 1; /**< APV Error */
        unsigned int pipelineAddr : 8; /**< Pipeline Address */
        unsigned int APVnum : 6; /**< APV chip number */
        unsigned int check : 2; /**< MSB "10" - for APV Header identification */
      };

      /** implementation of the first data word */
      struct data_A {
        unsigned int sample1 : 8; /**< 1st data sample */
        unsigned int sample2 : 8; /**< 2nd data sample */
        unsigned int sample3 : 8; /**< 3rd data sample */
        unsigned int stripNum : 7; /**< Strip number */
        unsigned int check : 1; /**< MSB "1" - for Data word identification */
      };

      /** implementation of the second data word */
      struct data_B {
        unsigned int sample4 : 8; /**< 4th data sample */
        unsigned int sample5 : 8; /**< 5th data sample */
        unsigned int sample6 : 8; /**< 6th data sample */
        unsigned int stripNum : 7; /**< Strip number */
        unsigned int check : 1; /**< MSB "1" - for Data word identification */
      };

      /** implementation of FADC Trailer */
      struct FADCTrailer {
        unsigned int FTBFlags: 16;  /**< FTB Flags Field */
        unsigned int dataSizeCut: 1; /**< APV data-size cut flag  */
        unsigned int nullDigits: 7; /**< "0000000" */
        unsigned int fifoErrOR: 1;  /**< FIFO full Error OR */
        unsigned int frameErrOR: 1; /**< Frame Error OR */
        unsigned int detectErrOR: 1;/**< Detection Error OR */
        unsigned int apvErrOR: 1;  /**< APV chip number OR */
        unsigned int check : 4; /**<  MSB "1110" - for FADC Trailer identification */
      };

      /** implementation of FTB Trailer */
      struct FTBTrailer {
        unsigned int crc16 : 16; /**< FTB CRC16 Checksum  */
        unsigned int controlWord : 16; /**< MSB "ff55" - FADC Trailer ID */
      };


      union {
        uint32_t data32; /**< Output 32-bit data word */
        FTBHeader m_FTBHeader; /**< Implementation of FTB Header */
        MainHeader m_MainHeader; /**< Implementation of FADC Header */
        APVHeader m_APVHeader; /**< Implementation of APV Header */
        data_A  m_data_A; /**< Implementation of 1st data word */
        data_B  m_data_B; /**< Implementation of 2nd data word */
        FADCTrailer m_FADCTrailer; /**< Implementation of FADC Trailer */
        FTBTrailer m_FTBTrailer; /**< Implementation of FTB Trailer */
      };

      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;   /**< Required input for EventMetaData */
      StoreObjPtr<SVDEventInfo> m_svdEventInfoPtr;  /**< SVDEventInfo from simulation */
      StoreArray<RawSVD> m_rawSVD;   /**< output for RawSVD */
      StoreArray<SVDShaperDigit> m_svdShaperDigit; /**< Required input for SVDShaperDigit */
      int m_FADCTriggerNumberOffset; /**< FADC trigger numnber offset*/

    };
  } //SVD
} // Belle2

#endif /* SVDPackerModule_H */
