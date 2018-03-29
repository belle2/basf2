/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/datastore/StoreArray.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPWaveformSegment.h>
#include <top/dataobjects/TOPProductionEventDebug.h>
#include <top/dataobjects/TOPProductionHitDebug.h>

#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPSlowData.h>
#include <top/dataobjects/TOPTemplateFitResult.h>
#include <string>

namespace Belle2 {

  namespace TOP {

    /**
     * Helper class for getting data words from a finesse buffer
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

      void resetChecksum() {m_checksumCounter = 0;}

      unsigned int foldChecksum()
      {
        unsigned int chk = m_checksumCounter;
        while ((chk >> 16) > 0) {
          chk = (chk & 0xFFFF) + (chk >> 16);
        }
        return chk;
      };

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

      unsigned int m_checksumCounter;

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
     * @param digits collection to unpack to
     */
    void unpackProductionDraft(const int* buffer, int bufferSize,
                               StoreArray<TOPDigit>& digits);

    /**
     * Unpack raw data given in feature-extraction production format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param rawDigits collection to unpack to
     * @param slowData collection to unpack to
     */
    void unpackType0Ver16(const int* buffer, int bufferSize,
                          StoreArray<TOPRawDigit>& rawDigits,
                          StoreArray<TOPSlowData>& slowData);


    /**
     * Unpack raw data given in feature-extraction interim format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param rawDigits collection to unpack feature-extracted data
     * @param waveforms collection to unpack waveforms
     * @param template fit result collection to unpack template fit data
     * @param pedestalSubtracted false for version 2, true for version 3
     * @return number of words remaining in data buffer
     */
    int unpackInterimFEVer01(const int* buffer, int bufferSize,
                             StoreArray<TOPRawDigit>& rawDigits,
                             StoreArray<TOPRawWaveform>& waveforms,
                             StoreArray<TOPTemplateFitResult>& templateFits,
                             bool pedestalSubtracted);

    /**
     * Unpack raw data given in waveform format (Kurtis packets - IRS3B)
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param waveforms collection to unpack to
     * @return number of words remaining in data buffer
     */
    int unpackWaveformsIRS3B(const int* buffer, int bufferSize,
                             StoreArray<TOPRawWaveform>& waveforms);

    /**
     * Unpack raw data given in waveform format (gigE format - IRSX)
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param waveforms collection to unpack to
     * @return number of words remaining in data buffer
     */
    int unpackWaveformsGigE(const int* buffer, int bufferSize,
                            StoreArray<TOPRawWaveform>& waveforms);

    /**
     * Unpack raw data given in production debugging format produced by TOP production firmware
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @return number of words remaining in data buffer
     */
    int unpackProdDebug(const int* buffer, int bufferSize, StoreArray<TOPRawDigit>& rawDigits,
                        StoreArray<TOPWaveformSegment>& prodWaveforms, StoreArray<TOPSlowData>& slowData,
                        StoreArray<TOPProductionEventDebug>& productionEventDebugs, StoreArray<TOPProductionHitDebug>& productionHitDebugs,
                        bool pedestalSubtracted = false);

    std::string m_inputRawDataName;  /**< name of RawTOP store array */
    std::string m_outputDigitsName;  /**< name of TOPDigit store array */
    std::string m_outputRawDigitsName;  /**< name of TOPRawDigit store array */
    std::string m_outputWaveformsName;  /**< name of TOPRawWaveform store array */
    std::string m_templateFitResultName; /**< name of TOPTemplateFitResult store array */

    std::map<int, int> m_channelStatistics; /**<counts how many different channels have been parsed in a given SCROD packet */

    bool m_swapBytes = false;     /**< if true, swap bytes */
    int m_dataFormat = 0;         /**< data format */

    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance(); /**< geometry param */

  };


} // Belle2 namespace
