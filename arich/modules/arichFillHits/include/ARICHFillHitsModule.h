/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef ARICHFILLHITSMODULE_H
#define ARICHFILLHITSMODULE_H

#include <arich/dbobjects/ARICHGeometryConfig.h>
#include <arich/dbobjects/ARICHChannelMapping.h>
#include <arich/dbobjects/ARICHChannelMask.h>


#include <framework/database/DBObjPtr.h>


#include <framework/core/Module.h>

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
     * Corrects hit position for distorsion due to non-perpendicular magnetic field component
     */
    void magFieldCorrection(TVector3& hitpos);

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

    DBObjPtr<ARICHGeometryConfig> m_geoPar; /**< geometry configuration parameters from the DB */
    DBObjPtr<ARICHChannelMapping> m_chnMap; /**< (x,y) to asic channel mapping */
    DBObjPtr<ARICHChannelMask> m_chnMask;     /**< list of dead channels from the DB */
    uint8_t m_bitMask; /**< hit bit mask (only convert digits with hit in bitmask bits) */
    uint8_t m_maxApdHits; /**< reject hits with more than number of hits in Apd */
    uint8_t m_maxHapdHits; /**< reject hits with more than number of hits in Hapd */
    int m_bcorrect = 0; /**< apply hit position correction for the non-perp. magnetic field component*/
    int m_fillall = 0; /**< make hit for all active channels (usefull for likelihood PDF studies) */
  };

} // Belle2 namespace

#endif
