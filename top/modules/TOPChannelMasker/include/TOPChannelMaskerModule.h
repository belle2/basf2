/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPAsicMask.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <top/dbobjects/TOPCalChannelMask.h>
#include <top/dbobjects/TOPPmtInstallation.h>
#include <top/dbobjects/TOPPmtQE.h>
#include <top/dbobjects/TOPCalChannelRQE.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>
#include <top/dbobjects/TOPCalChannelT0.h>
#include <top/dbobjects/TOPCalTimebase.h>

namespace Belle2 {
  /**
   * Masks dead PMs from the reconstruction
   *
   *    *
   */
  class TOPChannelMaskerModule : public Module {

  public:

    /**
     * Constructor: Sets the description of the module
     */
    TOPChannelMaskerModule();

    /**
     * initialize method: registers datastore objects (the TOP hits)
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     *
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * event method: removes channels from the reconstruction pdf, flags hits
     * from noisy channels as junk
     */
    virtual void event() override;

  private:

    // steering parameters
    bool m_maskUncalibratedChannelT0; /**< if true mask channelT0-uncalibrated channels */
    bool m_maskUncalibratedTimebase; /**< if true mask timebase-uncalibrated channels */

    // collections
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreObjPtr<TOPAsicMask> m_eventAsicMask; /**< masked asics in firmware */

    // database objects for masking
    TOPAsicMask m_savedAsicMask; /**< the default ones or a copy from data store */
    DBObjPtr<TOPCalChannelMask> m_channelMask; /**< list of dead/noisy channels */
    DBObjPtr<TOPCalChannelT0> m_channelT0; /**< channel T0 */
    DBObjPtr<TOPCalTimebase> m_timebase; /**< timebase */

    // database objects used only to check "hasChanged" status
    OptionalDBArray<TOPPmtInstallation> m_pmtInstalled; /**< PMT installation data */
    OptionalDBArray<TOPPmtQE> m_pmtQEData; /**< quantum efficiencies */
    DBObjPtr<TOPCalChannelRQE> m_channelRQE; /**< channel relative quantum effi. */
    DBObjPtr<TOPCalChannelThresholdEff> m_thresholdEff; /**< channel threshold effi. */

  };
}
