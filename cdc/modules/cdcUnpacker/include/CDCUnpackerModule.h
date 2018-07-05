/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCUnpackerModule_H
#define CDCUnpackerModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <cdc/dataobjects/CDCRawHitWaveForm.h>
#include <cdc/dataobjects/WireID.h>
#include <cdc/dbobjects/CDCChannelMap.h>
#include <cdc/dbobjects/CDCADCDeltaPedestals.h>
#include <rawdata/dataobjects/RawDataBlock.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawCDC.h>


namespace Belle2 {

  namespace CDC {

    /**
     * CDCUnpackerModule: The CDC Raw Hits Decoder.
     */

    class CDCUnpackerModule : public Module {

    public:
      /**
       * Constructor of the module.
       */
      CDCUnpackerModule();

      /**
       * Destructor of the module.
       */
      virtual ~CDCUnpackerModule();

      /**
       * Initializes the Module.
       */
      virtual void initialize();

      /**
       * Begin run action.
       */

      virtual void beginRun();

      /**
       * Event action (main routine).
       *
       */

      virtual void event();

      /**
       * End run action.
       */
      virtual void endRun();

      /**
       * Termination action.
       */
      virtual void terminate();

      /**
       * Set CDC Packet header
       */
      void setCDCPacketHeader(int* buf)
      {

        if ((buf[0] & 0xff000000) == 0x22000000) { // raw data mode.
          m_dataType = 1;
        } else if ((buf[0] & 0xff000000) == 0x20000000) { // suppressed data mode.
          m_dataType = 2;
        } else {
          B2ERROR("Undefined data type");
        }

        m_version = ((buf[0] & 0xff0000) >> 16); // Always zero.
        m_boardId = (buf[0] & 0xffff);
        m_triggerTime = ((buf[1] & 0xffff0000) >> 16);
        m_dataLength = (buf[1] & 0xffff);
        m_triggerNumber = buf[2];

      }

      /**
       * Getter for CDC data mode.
       * 1 for raw data mode, 2 for suppressed mode.
       */
      int getDataType()
      {
        return m_dataType;
      }

      /**
       * Getter for trigger time in nsec.
       */

      int getTriggerTime()
      {
        return m_triggerTime;
      }

      /**
       * Getter for data length in byte.
       */

      int getDataLength()
      {
        return m_dataLength;
      }

      /**
       * Getter for trigger number.
       */

      int getTriggerNumber()
      {
        return m_triggerNumber;
      }


      /**
       * Getter for FE board ID.
       */

      int getBoardId()
      {
        return m_boardId;
      }

      /**
       * Load FE channel to cell ID map.
       */
      void loadMap();

      /**
       * Set DBobject of ADC delta pedestal.
       */
      void setADCPedestal();


      /**
       * Getter of Wire ID.
       */
      WireID getWireID(int iBoard, int iCh) const;

      /**
       * Print out the CDC data block in hex.
       *  @param buf pointer to the buffer in RawCDC (RawCOPPER).
       *  @param nwords number of words to be printed out.
       */
      void printBuffer(int* buf, int nwords);

      /**
       * Check if the hit wire is valid or not.
       * @param WireID hit wire.
       */
      bool isValidBoardChannel(WireID wireId)
      {
        if (wireId.getEWire() == 65535) {
          return false;
        } else {
          return true;
        }
      }
    private:

      /**
       * Data type of CDC data block.
       */
      int m_dataType;

      /**
       * Format version.
       */
      int m_version;

      /**
       * Front end board ID.
       */
      int m_boardId;

      /**
       * Trigger time.
       */
      int m_triggerTime;

      /**
       * Data length of the CDC data block (in bytes).
       */
      int m_dataLength;

      /**
       * Trigger number.
       */
      int m_triggerNumber;

      /**
       * Enable/Disable to store CDCRawHit.
       */
      bool m_enableStoreCDCRawHit;

      /**
       * Enable/Disable to print out the data to the terminal.
       */

      bool m_enablePrintOut;


      /**
       * Name of the RawCDC dataobject (suppressed mode).
       */
      std::string m_rawCDCName;

      /**
       * Name of the CDCRawHit dataobject (suppressed mode).
       */
      std::string m_cdcRawHitName;

      /**
       * Name of the CDCRawHit dataobject (raw data mode).
       */
      std::string m_cdcRawHitWaveFormName;

      /**
       * Tree name of the CDCHit object.
       */
      std::string m_cdcHitName;

      /**
       * Name of the assignment map of FE board channel to the cell.
       */
      std::string m_xmlMapFileName;

      /**
       * Relation name between CDCRawHit and CDCHit.
       */
      std::string m_relCDCRawHitToCDCHitName;

      /**
       * Relation name between CDCRawHitWaveForm and CDCHit.
       */

      std::string m_relCDCRawHitWFToCDCHitName;

      /**
       * Short ward buffer of CDC event block.
       */
      std::vector<unsigned short> m_buffer;

      /**
       * Assignment map of FE board channel to the cell.
       * 1st column : board ID , 2nd column : FE channel (0-47).
       */
      WireID m_map[300][48];

      /**
       * FADC threshold.
       */
      int m_fadcThreshold;

      /**
       * TDC offset (nsec).
       */
      int m_tdcOffset;

      /**
       * TDC auxiliary offset (nsec).
       */
      int m_tdcAuxOffset;
      /**
       * Board ID for the trigger.
       */
      int m_boardIDTrig;

      /**
       * Channel for the trigger.
       */
      int m_channelTrig;

      /**
       * Enable/Disable to subtract the trigger timing
       * from TDCs.
       */
      bool m_subtractTrigTiming;

      /**
       * Enable/Disable to read the channel map
       * from the database.
       */
      bool m_enableDatabase;

      /**
       * Enable/Disable to 2nd hit output.
       *
       */
      bool m_enable2ndHit;

      /**
       * Channel map retrieved from DB.
       */
      DBArray<CDCChannelMap>* m_channelMapFromDB;
      // DBArray<CDCChannelMap> m_channelMapFromDB;

      /**
       * ADC delta pedestal.
       */
      DBObjPtr<CDCADCDeltaPedestals>* m_adcPedestalFromDB = nullptr;

      /**
       * Whether pedestal is subtracted (true) or not (false).
       */
      bool m_pedestalSubtraction = true;
      /**
       * Input array for CDC Raw.
       */
      StoreArray<RawCDC> m_rawCDCs;

    };//end class declaration


  } //end CDC namespace;
} // end namespace Belle2

#endif // CDCUnpackerModule_H

