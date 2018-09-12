/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Jan Strube (jan.strube@pnnl.gov)                                    *
 *    Sam Cunliffe (sam.cunliffe@desy.de)                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>     // database objects framwork
#include <top/dbobjects/TOPCalChannelMask.h> // Umberto's database object


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
    /** list of dead/noisy channels from the DB */
    DBObjPtr<TOPCalChannelMask> m_channelMask;
  };
}
