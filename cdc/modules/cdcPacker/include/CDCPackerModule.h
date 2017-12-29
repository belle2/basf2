/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Satoru Yamada and Makoto Uchida                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCPACKERMODULE_H
#define CDCPACKERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <rawdata/dataobjects/RawDataBlock.h>
#include <cdc/dbobjects/CDCChannelMap.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/database/DBArray.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawCDC.h>


namespace Belle2 {

  namespace CDC {

    /**
     * CDCPackerModule: The CDC Raw Hits Decoder.
     */

    class CDCPackerModule : public Module {

    public:
      /**
       * Constructor of the module.
       */
      CDCPackerModule();

      /**
       * Destructor of the module.
       */
      virtual ~CDCPackerModule();

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
       * Set CDC Packet header.
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
       * Getter for Front End Electronics ID.
       * @param copper_id Copper ID.
       * @param slod_id FINNES slot ID (0-3).
       */
      int getFEEID(int copper_id, int slot_id);

      /**
       * Getter for CDC data mode.
       * 1 for raw data mode, 2 for supressed mode.
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
       * Getter of Wire ID.
       */
      const WireID getWireID(int iBoard, int iCh);

    private:

      int m_event; /**< Event number. */
      int m_fadcThreshold;  /**< FADC threshold. */
      //      int m_overflow; /**< TDC overflow. */


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
       * Frontend board ID.
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

      ///**
      //       * Number of good blocks.
      //       */
      //            int m_nGoodBlocks;

      //      /**
      //       * Number of error blocks.
      //       */
      //      int m_nErrorBlocks;

      /**
       * Enable/Disable to store CDCRawHit.
       */
      bool m_enableStoreCDCRawHit;

      /**
       * Enable/Disable to print out the data to the terminal.
       */

      bool m_enablePrintOut;

      /**
       * Name of the RawCDC dataobject (supressed mode).
       */
      std::string m_rawCDCName;

      /**
       * Name of the CDCRawHit dataobject (supressed mode).
       */
      std::string m_cdcRawHitName;

      /**
       * Tree name of the CDCHit object.
       */
      std::string m_cdcHitName;

      /**
       * Name of the assignment map of FE board channel to the cell.
       */
      std::string m_xmlMapFileName;

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
       * Assignment map of FE board ID to the cell.
       * 1st column : Super Layer (0-8).
       * 2nd column : Local Layer (0-6 or 0-8).
       * 3rd column : Wire ID (phi).
       */
      int m_fee_board[9][8][384];

      /**
       * Assignment map of FE board channel to the cell.
       * 1st column : Super Layer (0-8).
       * 2nd column : Local Layer (0-6 or 0-8).
       * 3rd column : Wire ID (phi).
       */
      int m_fee_ch[9][8][384];

      /**
       * Enable/Disable to read the channel map
       * from the database.
       */
      bool m_enableDatabase;

      /**
       * Channel map retrieved from DB.
       */
      DBArray<CDCChannelMap> m_channelMapFromDB;

      /**
       * Raw CDC array.
       */
      StoreArray<RawCDC>  m_rawCDCs;
    };//end class declaration


  } //end CDC namespace;
} // end namespace Belle2

#endif // CDCPackerModule_H

