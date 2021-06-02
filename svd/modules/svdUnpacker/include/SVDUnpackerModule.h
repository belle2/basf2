/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski,  Giulia Casarosa, Eugenio Paoloni    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <svd/geometry/SensorInfo.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDTransparentDigit.h>
#include <svd/dataobjects/SVDDAQDiagnostic.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/dataobjects/SVDTriggerType.h>
#include <svd/dbobjects/SVDGlobalConfigParameters.h>
#include <svd/online/SVDOnlineToOfflineMap.h>
#include <svd/online/SVDStripNoiseMap.h>
#include <framework/dataobjects/EventMetaData.h>
#include <memory>
#include <framework/database/PayloadFile.h>

namespace Belle2::SVD {

  /** SVDUnpackerModule: The SVD Raw Hits Decoder.
   *
   * This module produces SVDShaperDigits from the Copper
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
    virtual void initialize() override; /**<initialize*/
    virtual void beginRun() override; /**<begin run*/
    virtual void event() override; /**<event*/
    virtual void endRun() override; /**<end run*/

    std::string m_rawSVDListName; /**<RawSVD StoreArray name*/
    std::string m_svdShaperDigitListName; /**<SVDShaperDigit StoreArray name*/
    std::string m_svdDAQDiagnosticsListName; /**<SVDDAQDiagnostic StoreArray name*/
    std::string m_svdEventInfoName; /**< SVDEventInfo name */

    int m_wrongFTBcrc; /**<FTB CRC no-Match counter*/


  private:

    /** how many FADCs we have */
    unsigned short nFADCboards;

    /** pointer to APVforFADCmap filled by mapping procedure */
    std::unordered_multimap<unsigned char, unsigned char>* APVmap;

    /** Pointer to online-to-offline map */
    std::unique_ptr<SVDOnlineToOfflineMap> m_map;

    /** XML filename */
    static std::string m_xmlFileName;

    /** pointer to the payload with the mapping */
    DBObjPtr<PayloadFile> m_mapping;

    /** instance of SVDModeByte for the event */
    SVDModeByte m_SVDModeByte;

    /** additional function that prints raw data words */
    void printB2Debug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords);

    // The following assumes i386 byte order: MSB comes last!

    /** implementation of FTB Header */
    struct FTBHeader {
      unsigned int errorsField : 8;  /**< FTB error fields */
      unsigned int eventNumber : 24; /**< FTB event number */
    };

    /** implementation of FADC Header */
    struct MainHeader {
      unsigned int trgNumber : 8; /**< Trigger Number */
      unsigned int trgType   : 4; /**< Trigger Type */
      unsigned int trgTiming : 2; /**< Trigger Timing */
      unsigned int xTalk     : 2; /**< cross talk tag */
      unsigned int FADCnum   : 8; /**< FADC number */
      unsigned int DAQType   : 1; /**< (from 2020c) Event type(0): "0"…3 or …6 acquisition mode, "1"…3-mixed-6 acquisition mode */
      unsigned int DAQMode   : 2; /**< Event type(2:1): "00"…1-sample, "01"…3-sample, "10"…6-sample */
      unsigned int runType   : 2; /**< Run Type */
      unsigned int check     : 3; /**< MSB "110" - for FADC Header identification */
    };

    /** implementation of APV Header */
    struct APVHeader {
      unsigned int CMC1      : 8; /**< Common Mode Noise w/o masking out particle signals */
      unsigned int CMC2      : 4; /**< Common Mode Noise after masking out particle signals */
      unsigned int apvErr    : 4; /**< APV Errors field */
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
      unsigned int FTBFlags: 16; /**< FTB Flags Field */
      unsigned int dataSizeCut: 1; /**< APV data-size cut flag  */
      unsigned int nullDigits: 7; /**< "0000000" */
      unsigned int apvErrOR  : 4; /**< APV Errors Field OR*/
      unsigned int check : 4; /**<  MSB "1110" - for FADC Trailer identification */
    };

    /** implementation of FTB Trailer */
    struct FTBTrailer {
      unsigned int crc16 : 16; /**< FTB CRC16 Checksum  */
      unsigned int controlWord : 16; /**< MSB "ff55" - FADC Trailer ID */
    };


    union {  // The 4 byte words of the stream can be interpreted as:
      uint32_t m_data32; /**< Input 32-bit data word */
      FTBHeader m_FTBHeader; /**< Implementation of FTB Header */
      MainHeader m_MainHeader;  /**< Implementation of FADC Header */
      APVHeader m_APVHeader;  /**< Implementation of APV Header */
      data_A  m_data_A; /**< Implementation of 1st data word */
      data_B  m_data_B; /**< Implementation of 2nd data word */
      FADCTrailer m_FADCTrailer;  /**< Implementation of FADC Trailer */
      FTBTrailer m_FTBTrailer; /**< Implementation of FTB Trailer */
    };

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;   /**< Required input for EventMetaData */
    StoreObjPtr<SVDEventInfo> m_svdEventInfoPtr;  /**< SVDEventInfo output per event */
    SVDTriggerType m_SVDTriggerType;  /**< SVDTriggerType object */

    StoreArray<RawSVD> m_rawSVD;   /**< output for RawSVD */
    StoreArray<SVDDAQDiagnostic> m_storeDAQDiagnostics; /**< SVDDAQDiagnostic array */
    StoreArray<SVDShaperDigit> m_storeShaperDigits; /**< SVDShaperDigit array */

    int m_shutUpFTBError; /**< regulates the number of "Event number mismatch" errors reported */
    int m_FADCTriggerNumberOffset; /**< FADC Trigger Offset */

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

    /** Silently append new SVDShaperDigits to a pre-existing non-empty
     * SVDShaperDigits storeArray.
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

    /** Optionally we can get printout of Raw Data words */
    bool m_printRaw;

    /** The parameter that indicates what fraction of B2ERRORs messages
     * should be suppressed to not overload HLT while data taking
     */
    int m_errorRate{1000};

    /** this 4-bits value should be 1111 if no headers/trailers are missing */
    unsigned short seenHeadersAndTrailers: 4;

    /** counters for specific ERRORS produced by the Unpacker */
    int nTriggerMatchErrors; /**< counter of Trigger match errors */
    int nEventMatchErrors; /**< counter of Event match errors */
    int nUpsetAPVsErrors; /**< counter of upset APV errors */
    int nErrorFieldErrors; /**< counter of event mismatch errors in FTB's ErrorField */
    int nMissingAPVsErrors; /**< counter of missing APVs errors*/
    int nFADCMatchErrors; /**< counter of FADC boards =/= n of RawData objects errors */
    int nAPVErrors; /**< counter of APV errors*/
    int nFTBFlagsErrors; /**< counter of errors in FTBFlags variable */
    int nEventInfoMatchErrors; /**< counter of inconsistencies in SVDEventInfo within an event */

    /** Map to store a list of missing APVs */
    std::map<std::pair<unsigned short, unsigned short>, std::pair<std::size_t, std::size_t> > m_missingAPVs;

    /** Map to store a list of upset APVs */
    std::map<std::pair<unsigned short, unsigned short>, std::pair<std::size_t, std::size_t> > m_upsetAPVs;

    int m_relativeTimeShift; /**< latency difference between the 3- and 6-sample acquired events in usint of APV clock / 4, read from SVDGlobalConfigParameters and filled into SVDEventInfo */

    DBObjPtr<SVDGlobalConfigParameters> m_svdGlobalConfig;  /**< SVDGlobal Configuration payload*/

  };//end class declaration

}


