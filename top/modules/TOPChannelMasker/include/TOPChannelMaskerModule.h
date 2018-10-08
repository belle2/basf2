/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Jan Strube (jan.strube@pnnl.gov)                                    *
 *    Sam Cunliffe (sam.cunliffe@desy.de)                                 *
 *    Marko Staric                                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h> // data store framework
#include <top/dataobjects/TOPDigit.h>       // data Cherenkov hits
#include <framework/database/DBObjPtr.h>     // database objects framwork
#include <framework/database/DBArray.h>
#include <top/dbobjects/TOPCalChannelMask.h> // Umberto's database object
#include <top/dbobjects/TOPPmtInstallation.h>
#include <top/dbobjects/TOPPmtQE.h>
#include <top/dbobjects/TOPCalChannelRQE.h>
#include <top/dbobjects/TOPCalChannelThresholdEff.h>

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
    virtual void initialize();

    /**
     * event method: removes channels from the reconstruction pdf, flags hits
     * from noisy channels as junk
     */
    virtual void event();

  private:

    StoreArray<TOPDigit> m_digits; /**< collection of digits */

    DBObjPtr<TOPCalChannelMask> m_channelMask; /**< list of dead/noisy channels */

    // those below are used only to check "hasChanged" status
    DBArray<TOPPmtInstallation> m_pmtInstalled; /**< PMT installation data */
    DBArray<TOPPmtQE> m_pmtQEData; /**< quantum efficiencies */
    DBObjPtr<TOPCalChannelRQE> m_channelRQE; /**< channel relative quantum effi. */
    DBObjPtr<TOPCalChannelThresholdEff> m_thresholdEff; /**< channel threshold effi. */

  };
}
