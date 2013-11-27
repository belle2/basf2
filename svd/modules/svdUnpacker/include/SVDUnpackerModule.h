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
#include <daq/dataobjects/RawSVD.h>
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

    private:

      SVDOnlineToOfflineMap* m_map;

      struct MainHeader { //from less to most significant
        unsigned int check : 3; //LSB
        unsigned int runType : 2;
        unsigned int evtType : 3;
        unsigned int FADCnum : 8;
        unsigned int trgTiming : 8;
        unsigned int trgNumber : 8; //MSB
      };

      struct APVHeader {
        unsigned int check : 2;
        unsigned int APVnum : 6;
        unsigned int pipelineAddr : 8;
        unsigned int errorBit : 1;
        unsigned int reserved : 2;
        unsigned int CMC2 : 4;
        unsigned int CMC1 : 8;
      };

      struct data {
        unsigned int check : 1;
        unsigned int stripNum : 7;
        unsigned int sample2 : 8;
        unsigned int sample1 : 8;
        unsigned int sample0 : 8;
      };

      struct trailer {
        unsigned int check : 4;
        unsigned int error2: 1;
        unsigned int error1: 1;
        unsigned int error0: 1;
        unsigned int wiredOrErr: 1;
        unsigned int emPipeAddr: 8;
        unsigned int checksum: 16;
      };


      void checksum();

      void loadMap();

      void fillSVDDigitList(uint32_t* data32, StoreArray<SVDDigit>* svdDigits);


    };//end class declaration

    /** @}*/

  } //end SVD namespace;
} // end namespace Belle2

#endif // SVDUnpackerModule_H

