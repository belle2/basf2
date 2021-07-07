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
#include <rawdata/dataobjects/RawFTSW.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

namespace Belle2 {

  /** TTD Unpacker module.
   *
   * Read TTD data object and fill mdst objects
   */
  class TTDUnpackerModule : public Module {

  public:

    /** Constructor defining the parameters */
    TTDUnpackerModule();

  private:
    /** Input array for TTD/FTSW */
    StoreArray<RawFTSW> m_rawTTD;
    /** Output object for TTD mdst object */
    StoreObjPtr<EventLevelTriggerTimeInfo> m_EventLevelTriggerTimeInfo;

    void initialize() override final; /**< initialize function */

    void event() override final; /**< event function */

  };//end class declaration


} // end namespace Belle2
