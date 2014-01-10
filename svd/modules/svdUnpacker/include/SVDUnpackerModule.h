/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDUnpackerModule_H
#define SVDUnpackerModule_H

#include <framework/core/Module.h>

#include <svd/geometry/SensorInfo.h>

#include <framework/datastore/StoreArray.h>
#include <vxd/dataobjects/VxdID.h>
//disappeared from the release:
#include <rawdata/dataobjects/RawSVD.h>
#include <svd/dataobjects/SVDDigit.h>

#include <svd/online/SVDOnlineToOfflineMap.h>

namespace Belle2 {

  namespace SVD {
    /** \addtogroup modules
     * @{
     */

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

      /**
       * Termination action.
       */
      virtual void terminate();

      std::string m_rawSVDListName;
      std::string m_svdDigitListName;
      std::string m_xmlMapFileName;

      float m_APVLatency;
      float m_APVSamplingTime;
      int m_failedChecks;
      int m_wrongFTBHeader;
      int m_noAPVHeader;
      int m_noNewDigit;
      int m_NewDigit;
      int m_wrongFADCHeader;
      int m_wrongRunType;
      int m_wrongFADCTrailer;
      int m_wrongFADCcrc;
      int m_badEvent;
      int m_wrongFTBtrailer;
      int m_f0;
      int m_f3;
      int m_f5;
      int m_f6;
      int m_f7;

    private:

      SVDOnlineToOfflineMap* m_map;

      // The following assumes i386 byte order: MSB comes last!

      struct FTBHeader {
        unsigned int controlWord : 32; //LSB
        unsigned int errorsField : 8;
        unsigned int eventNumber : 24; //MSB
      };


      struct MainHeader {
        unsigned int trgNumber : 8; //LSB
        unsigned int trgTiming : 8;
        unsigned int FADCnum : 8;
        unsigned int evtType : 3;
        unsigned int runType : 2;
        unsigned int check : 3; //MSB
      };

      struct APVHeader {
        unsigned int CMC1 : 8; //LSB

        unsigned int CMC2 : 4;
        unsigned int reserved : 3;
        unsigned int errorBit : 1;

        unsigned int pipelineAddr : 8;

        unsigned int APVnum : 6;
        unsigned int check : 2; //MSB
      };

      struct data {
        unsigned char sample[3]; //LSB
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


      bool sanityChecks(int nWords, uint32_t* data32);

      bool verifyFTBcrc();

      void loadMap();

      void fillSVDDigitList(int nWords, uint32_t* data32_in, StoreArray<SVDDigit>* svdDigits);

      void printDebug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords);

      //temporary: to check format from Vienna test files:
      //      void printbitssimple(int n, int nBits);

    };//end class declaration

    /** @}*/

  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDUnpackerModule_H

