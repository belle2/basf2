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

    std::string m_outputDigitsName;   /**< name of ARICHDigit store array */
    std::string m_inputRawDataName; /**< name of RawARICH store array */

    DBObjPtr<ARICHMergerMapping> m_mergerMap; /**< mapping of modules to mergers */

  };

} // Belle2 namespace

#endif
