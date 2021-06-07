/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    std::string m_EventLevelTriggerTimeInfoName;  /**< The name of the StoreArray of filled EventLevelTriggerTimeInfo */

    void initialize() override final; /**< initialize function */

    void event() override final; /**< event function */

  };//end class declaration


} // end namespace Belle2
