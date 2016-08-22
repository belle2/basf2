/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHPACKERMODULE_H
#define ARICHPACKERMODULE_H

#include <framework/core/Module.h>
#include <arich/geometry/ARICHGeometryPar.h>
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

    /**
     * Writes raw data header
     * @param buffer pointer to data buffer
     * @param ibyte position of first header byte in buffer
     * @param head raw data header
     */
    void writeHeader(int* buffer, unsigned& ibyte, const ARICHRawHeader& head);


  private:

    ARICHGeometryPar* m_arichgp;  /**< geometry parameters */

    unsigned m_nonSuppressed;   /**< type of data (1 nonsuppressed, 0 suppressed) */
    unsigned m_version;
    unsigned m_bitMask;    /**< bitmask for hit detection (4bit/hit) */
    int m_debug; /**< debug */
    std::string m_inputDigitsName;   /**< name of ARICHDigit store array */
    std::string m_outputRawDataName; /**< name of RawARICH store array */
  };

} // Belle2 namespace

#endif
