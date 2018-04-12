/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - 2018 - Belle II Collaboration                      *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric,  Oskar Hartbrich                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/datastore/StoreArray.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPSlowData.h>
#include <top/dataobjects/TOPInterimFEInfo.h>
#include <top/dataobjects/TOPTemplateFitResult.h>
#include <top/dataobjects/TOPProductionEventDebug.h>
#include <top/dataobjects/TOPProductionHitDebug.h>
#include <string>

namespace Belle2 {

  namespace TOP {

    /**
     * Helper class for getting data words from a finesse buffer
     * Keeps checksum counter for each extracted data word and calculates tcp checksum on request
     */
    class DataArray {

    public:

      /**
       * Constructor
       * @param data pointer to finesse buffer (pointer must be valid)
       * @param size buffer size
       * @param swap if set to true, swap bytes
       */
      DataArray(const int* data, int size, bool swap = false)
      {
        m_data = data;
        m_size = size;
        m_i = -1;
        m_swap = swap;
        m_checksumCounter = 0;
      }

      /**
       * Returns consecutive data word
       * Updates internal checksum counter for each extracted word.
       * Verify checksum using DataArray::validateChecksum()
       * @return data word
       */
      int getWord()
      {
        m_i++;
        if (m_i < m_size) {
          m_checksumCounter += (m_data[m_i] & 0xFFFF) + ((m_data[m_i] >> 16) & 0xFFFF);
          if (m_swap) return swap32(m_data[m_i]);
          return m_data[m_i];
        }
        B2ERROR("Bug in data format: DataArray - index out of range");
        return 0;
      }

      /**
       * Returns next data word without incrementing the memory pointer and without modifying the checksum counter
       * @return data word
       */
      int peekWord()
      {
        if (m_i + 1 < m_size) {
          if (m_swap) return swap32(m_data[m_i + 1]);
          return m_data[m_i + 1];
        }

        return 0;
      }

      /**
       * Returns last data word
       * @return last data word
       */
      int getLastWord()
      {
        if (m_size <= 0) return 0;
        if (m_swap) return swap32(m_data[m_size - 1]);
        return m_data[m_size - 1];
      }

      /**
       * Returns index of last returned data word
       * @return index
       */
      unsigned int getIndex() const {return m_i;}

      /**
       * Returns number of remaining words in the buffer
       * @return number of remaining words
       */
      int getRemainingWords() const {return m_size - m_i - 1;}

      /**
       * Resets internal checksum counter.
       * Call at the beginning of each new block that is checksummed.
       */
      void resetChecksum() {m_checksumCounter = 0;}

      /**
       * Performs folding of carry bits into checksum until only 16 LSBs are populated.
       * @return reduced checksum counter
       */
      unsigned int foldChecksum()
      {
        unsigned int chk = m_checksumCounter;
        while ((chk >> 16) > 0) {
          chk = (chk & 0xFFFF) + (chk >> 16);
        }
        return chk;
      };

      /**
       * Validates current checksum counter status.
       * Call at the end of each block that is checksummed.
       * @return checksum status, true if checksum is correct.
       */
      bool validateChecksum()
      {
        return (foldChecksum() == 0xFFFF);
      }

    private:

      /**
       * Swap bytes of a 32-bit integer
       */
      int swap32(int x)
      {
        return (((x << 24) & 0xFF000000) |
                ((x <<  8) & 0x00FF0000) |
                ((x >>  8) & 0x0000FF00) |
                ((x >> 24) & 0x000000FF));
      }

      int m_size = 0;        /**< buffer size */
      const int* m_data = 0; /**< buffer data */
      int m_i = 0;           /**< index */
      bool m_swap = false;   /**< if true, swap bytes */
      unsigned int m_checksumCounter; /**< check sum counter */

    };

  } // TOP namespace


  /**
   * Raw data unpacker
   */
  class TOPUnpackerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPUnpackerModule();

    /**
     * Destructor
     */
    virtual ~TOPUnpackerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Expand 13-bit signed-word to 16-bit signed-word
     */
    short expand13to16bits(unsigned short x) const
    {
      unsigned short signBit = x & 0x1000;
      return ((x & 0x1FFF) | signBit << 1 | signBit << 2 | signBit << 3);
    }


    /**
     * sum both 16-bit words of 32-bit integer
     */
    unsigned short sumShorts(unsigned int x) const
    {
      return x + (x >> 16);
    }


    /**
     * Unpack raw data given in a tentative production format (will vanish in future)
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     */
    void unpackProductionDraft(const int* buffer, int bufferSize);

    /**
     * Unpack raw data given in feature-extraction production format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     */
    void unpackType0Ver16(const int* buffer, int bufferSize);

    /**
     * Unpack raw data given in feature-extraction interim format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param pedestalSubtracted false for version 2, true for version 3
     * @return number of words remaining in data buffer
     */
    int unpackInterimFEVer01(const int* buffer, int bufferSize, bool pedestalSubtracted);

    /**
     * Tries to unpack raw data assuming it is in feature-extraction interim format.
     * Does not write out anything, just checks integrity.
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param swapBytes if true, swap bytes in buffer
     * @return true if buffer resembles interim format, false if not.
     */
    bool unpackHeadersInterimFEVer01(const int* buffer, int bufferSize, bool swapBytes);

    /**
     * Unpack raw data given in production debugging format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param pedestalSubtracted true, if pedestal is subtracted in waveforms
     * @return number of words remaining in data buffer
     */
    int unpackProdDebug(const int* buffer, int bufferSize, bool pedestalSubtracted);

    std::string m_inputRawDataName;  /**< name of RawTOP store array */
    std::string m_outputDigitsName;  /**< name of TOPDigit store array */
    std::string m_outputRawDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_outputWaveformsName;  /**< name of TOPRawWaveform store array */
    std::string m_templateFitResultName; /**< name of TOPTemplateFitResult store array */

    std::map<int, int> m_channelStatistics; /**<counts how many different channels have been parsed in a given SCROD packet */

    bool m_swapBytes = false;     /**< if true, swap bytes */
    int m_dataFormat = 0;         /**< data format */

    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance(); /**< geometry param */

    // collections
    StoreArray<RawTOP> m_rawData;  /**< collection of raw data */
    StoreArray<TOPDigit> m_digits;   /**< collection of digits */
    StoreArray<TOPRawDigit> m_rawDigits;   /**< collection of raw digits */
    StoreArray<TOPSlowData> m_slowData;   /**< collection of slow data */
    StoreArray<TOPRawWaveform> m_waveforms;   /**< collection of waveforms */
    StoreArray<TOPInterimFEInfo> m_interimFEInfos;   /**< collection of interim informations */
    StoreArray<TOPProductionEventDebug> m_productionEventDebugs;   /**< collection of event debug data */
    StoreArray<TOPProductionHitDebug> m_productionHitDebugs;   /**< collection of hit debug data */
    StoreArray<TOPTemplateFitResult> m_templateFitResults;   /**< collection of template fit results */

  };

} // Belle2 namespace
