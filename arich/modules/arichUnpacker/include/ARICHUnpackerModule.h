/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHUNPACKERMODULE_H
#define ARICHUNPACKERMODULE_H

// mapper
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <framework/database/DBObjPtr.h>

#include <framework/core/Module.h>
#include <arich/modules/arichUnpacker/ARICHRawDataHeader.h>
#include <string>

#include <TH1F.h>

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
     * Make histograms of the channel ID
     */
    virtual void defineHisto();

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
     */

    void printBits(const int* buffer, int bufferSize);
    void readHeader(const int* buffer, unsigned& ibyte, ARICHRawHeader& head);



    uint8_t m_bitMask; /**< bitmask for hit detection (8bits/hit) */
    int m_debug; /**< debug */

    int m_rawmode; /**< Unpacker mode */

    std::string m_outputDigitsName;   /**< name of ARICHDigit store array */
    std::string m_outputRawDigitsName;   /**< name of ARICHRawDigit store array */
    std::string m_outputarichinfoName;   /**< name of ARICHInfo store array */
    std::string m_inputRawDataName; /**< name of RawARICH store array */

    DBObjPtr<ARICHMergerMapping> m_mergerMap; /**< mapping of modules to mergers */

  protected:
    unsigned int calbyte(const int* buf);
    unsigned int calword(const int* buf);
    unsigned int m_ibyte;
    TH1* h_rate_a_all;//yone
    TH1* h_rate_b_all;//yone
    TH1* h_rate_c_all;//yone
    TH1* h_rate_d_all;//yone
  };

  inline unsigned int ARICHUnpackerModule::calbyte(const int* buf)
  {
    int shift = (3 - m_ibyte % 4) * 8;
    unsigned int val = 0xff & (buf[m_ibyte / 4] >> shift);
    m_ibyte++;
    return val;
  }

  inline unsigned int ARICHUnpackerModule::calword(const int* buf)
  {
    return (calbyte(buf) << 24) | (calbyte(buf) << 16)
           | (calbyte(buf) << 8) | calbyte(buf);
  }

} // Belle2 namespace

#endif
