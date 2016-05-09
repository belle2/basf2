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
       */
      DataArray(const int* data, int size)
      {
        m_data = data;
        m_size = size;
        m_i = -1;
      }

      /**
       * Returns consecutive data word
       * @return data word
       */
      int getWord()
      {
        m_i++;
        if (m_i < m_size) {
          return m_data[m_i];
        }
        B2ERROR("Bug in data format: DataArray - index out of range");
        return 0;
      }

      /**
       * Returns index of last returned data word
       * @return index
       */
      int getIndex() const {return m_i;}

      /**
       * Returns number of remaining words in the buffer
       * @return number of remaining words
       */
      int getRemainingWords() const {return m_size - m_i - 1;}

    private:

      int m_size;        /**< buffer size */
      const int* m_data; /**< buffer data */
      int m_i;           /**< index */

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
     * Unpack raw data given in production format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param digits collection to unpack to
     */
    void unpackProductionFormat(const int* buffer, int bufferSize,
                                StoreArray<TOPDigit>& digits);

    /**
     * Unpack raw data given in waveform format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     * @param waveforms collection to unpack to
     */
    void unpackWaveformFormat(const int* buffer, int bufferSize,
                              StoreArray<TOPRawWaveform>& waveforms);

    /**
     * Unpack raw data given in waveform format version 1 (Kurtis packets - IRS3B)
     * @param array raw data buffer
     * @param feemap front-end map
     * @param waveforms collection to unpack to
     * @return number of words remaining in data buffer
     */
    int unpackWaveformFormatV1(TOP::DataArray& array,
                               const TOPFrontEndMap* feemap,
                               StoreArray<TOPRawWaveform>& waveforms);

    /**
     * Unpack raw data given in waveform format version 2 (gigE format - IRSX)
     * @param array raw data buffer
     * @param feemap front-end map
     * @param waveforms collection to unpack to
     * @return number of words remaining in data buffer
     */
    int unpackWaveformFormatV2(TOP::DataArray& array,
                               const TOPFrontEndMap* feemap,
                               StoreArray<TOPRawWaveform>& waveforms);

    std::string m_inputRawDataName;  /**< name of RawTOP store array */
    std::string m_outputDigitsName;  /**< name of TOPDigit store array */
    std::string m_outputWaveformsName;  /**< name of TOPRawWaveform store array */
    TOP::TOPGeometryPar* m_topgp = TOP::TOPGeometryPar::Instance(); /**< geometry param */

  };


} // Belle2 namespace

