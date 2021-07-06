/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHUNPACKERMODULE_H
#define ARICHUNPACKERMODULE_H

// mapper
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <framework/database/DBObjPtr.h>

#include <framework/core/Module.h>
#include <arich/modules/arichUnpacker/ARICHRawDataHeader.h>
#include <string>

namespace Belle2 {

  /**
   * Raw data unpacker for ARICH
   */
  class ARICHUnpackerModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHUnpackerModule();

    /**
     * Destructor
     */
    virtual ~ARICHUnpackerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

  private:

    /**
     * Unpack raw data given in production format
     * @param buffer raw data buffer
     * @param bufferSize buffer size
     */

    void printBits(const int* buffer, int bufferSize);
    void readHeader(const int* buffer, unsigned& ibyte, ARICHRawHeader& head); /** read Merger header */
    void readFEHeader(const int* buffer, unsigned& ibyte, ARICHRawHeader& head); /** read FE header */



    uint8_t m_bitMask; /**< bitmask for hit detection (8bits/hit) */
    int m_debug; /**< debug */

    int m_rawmode; /**< Activate Raw Unpacker */
    int m_disable_unpacker; /**< Disable regular Unpacker */

    std::string m_outputDigitsName;   /**< name of ARICHDigit store array */
    std::string m_outputRawDigitsName;   /**< name of ARICHRawDigit store array */
    std::string m_outputarichinfoName;   /**< name of ARICHInfo store object */
    std::string m_inputRawDataName; /**< name of RawARICH store array */

    DBObjPtr<ARICHMergerMapping> m_mergerMap; /**< mapping of modules to mergers */

  protected:
    unsigned int calbyte(const int* buf); /**< calculate number of bytes in raw Unpacker */
    unsigned int cal2byte(const int* buf); /**< calculate number of lines (2 bytes) in raw Unpacker */
    unsigned int calword(const int* buf); /**< calculate number of words in raw Unpacker */
    unsigned int m_ibyte = 0; /**< bye index of raw unpacker */
  };

  /**
   * calculate number of bytes in raw Unpacker
   */
  inline unsigned int ARICHUnpackerModule::calbyte(const int* buf)
  {
    int shift = (3 - m_ibyte % 4) * 8;
    unsigned int val = 0xff & (buf[m_ibyte / 4] >> shift);
    m_ibyte++;
    return val;
  }

  /**
   * calculate number of lines (2 bytes) in raw Unpacker
   */
  inline unsigned int ARICHUnpackerModule::cal2byte(const int* buf)
  {
    return (calbyte(buf) << 8) | calbyte(buf);
  }

  /**
   * calculate number of words in raw Unpacker
   */
  inline unsigned int ARICHUnpackerModule::calword(const int* buf)
  {
    return (calbyte(buf) << 24) | (calbyte(buf) << 16)
           | (calbyte(buf) << 8) | calbyte(buf);
  }

} // Belle2 namespace

#endif
