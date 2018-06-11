/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHFILLHITSMODULE_H
#define ARICHFILLHITSMODULE_H

#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHChannelMask.h>


#include <framework/database/DBObjPtr.h>


#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  /**
   * Fill ARICHHit collection from ARICHDigits
   */
  class ARICHFillHitsModule : public Module {

  public:

    /**
     * Constructor
     */
    ARICHFillHitsModule();

    /**
     * Destructor
     */
    virtual ~ARICHFillHitsModule();

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

    DBObjPtr<ARICHGeometryConfig> m_geoPar; /**< geometry configuration parameters from the DB */
    DBObjPtr<ARICHChannelMapping> m_chnMap; /**< (x,y) to asic channel mapping */
    DBObjPtr<ARICHChannelMask> m_chnMask;     /**< list of dead channels from the DB */
    uint8_t m_bitMask; /**< hit bit mask (only convert digits with hit in bitmask bits) */
    uint8_t m_maxApdHits; /**< reject hits with more than number of hits in Apd */
    uint8_t m_maxHapdHits; /**< reject hits with more than number of hits in Hapd */

  };

} // Belle2 namespace

#endif
