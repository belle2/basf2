/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHPACKERMODULE_H
#define ARICHPACKERMODULE_H

// mappers
#include <arich/dbobjects/ARICHMergerMapping.h>
#include <arich/dbobjects/ARICHCopperMapping.h>

#include <framework/database/DBObjPtr.h>

#include <framework/core/Module.h>
#include <string>
#include <arich/modules/arichUnpacker/ARICHRawDataHeader.h>

namespace Belle2 {

  /**
   * Raw data packer for ARICH
   */
  class ARICHPackerModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHPackerModule();

    /**
     * Destructor
     */
    virtual ~ARICHPackerModule();

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

    void writeHeader(int* buffer, unsigned& ibyte, const ARICHRawHeader& head);

  private:

    unsigned m_nonSuppressed;   /**< type of data (1 nonsuppressed, 0 suppressed) */
    unsigned m_version;
    unsigned m_bitMask;    /**< bitmask for hit detection (4bit/hit) */
    int m_debug; /**< debug */
    std::string m_inputDigitsName;   /**< name of ARICHDigit store array */
    std::string m_outputRawDataName; /**< name of RawARICH store array */

    DBObjPtr<ARICHMergerMapping> m_mergerMap; /**< mapping of modules to mergers */
    DBObjPtr<ARICHCopperMapping> m_copperMap; /**< mapping of mergers to coppers */

  };

} // Belle2 namespace

#endif
