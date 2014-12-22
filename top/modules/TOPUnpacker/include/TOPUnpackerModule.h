/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPUNPACKERMODULE_H
#define TOPUNPACKERMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>

namespace Belle2 {

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
     */
    void unpackProductionFormat(const int* buffer, int bufferSize);

    TOP::TOPGeometryPar* m_topgp;  /**< geometry parameters */

  };

} // Belle2 namespace

#endif
