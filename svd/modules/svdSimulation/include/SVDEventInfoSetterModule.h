/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#ifndef SVDEventInfoSetterModule_H
#define SVDEventInfoSetterModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <svd/dataobjects/SVDEventInfo.h>
#include <svd/dataobjects/SVDModeByte.h>
#include <svd/dataobjects/SVDTriggerType.h>
#include <simulation/dataobjects/SimClockState.h>
#include <svd/dbobjects/SVDGlobalConfigParameters.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/HardwareClockSettings.h>


namespace Belle2 {
  /**
   * Module to set the SVDEventInfo in the simulation
   */

  class SVDEventInfoSetterModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    SVDEventInfoSetterModule();

    /** Destructor. */
    virtual ~SVDEventInfoSetterModule();

    /** Initializes the Module.
     */
    virtual void initialize() override;

    /** Reads Detector Configuration from DB
     *  daqMode and relativeShift
     */
    virtual void beginRun() override;

    /** Stores the SVD event info into the DataStore.
     *
     * Based on the parameters set by the user the current SVD event info
     * is calculated and stored into the DataStore.
     */
    virtual void event() override;

    std::string m_svdEventInfoName; /**< Name of the SVDEventInfo object */

    /** Name of the StoreObjectPrt TRGSummary */
    std::string m_objTrgSummaryName = "TRGSummary";

    bool m_useDB; /**<if true reads the configuration from SVDGlobalConfigParameters payload**/

  private:
    StoreObjPtr<SVDEventInfo> m_svdEventInfoPtr; /**< Output object. */
    StoreObjPtr<SimClockState> m_simClockState; /**< generated hardware clock state */
    DBObjPtr<HardwareClockSettings> m_hwClock; /**< systems clock*/

    // SVDModeByte content */
    int m_runType; /**< run type */
    int m_eventType; /**< event type */
    int m_daqMode; /**< DAQ mode */
    int m_triggerBin = -1; /**< trigger bin */
    int m_fixedTriggerBin = 999; /**< fixed trigger bin */
    uint8_t m_triggerType; /**<  Trigger type content to be set */

    bool m_xTalk; /**<  Cross-talk flag to be set */

    bool m_ModeByteMatch = true; /**< SVDModeByte matching flag to be set */
    bool m_TriggerTypeMatch = true; /**< SVDTriggerType matching flag to be set */

    SVDModeByte m_SVDModeByte;  /**<  SVDModeByte object */
    SVDTriggerType m_SVDTriggerType;  /**<  SVDTriggerType object */

    int m_relativeShift; /**< latency difference between the 3- and 6-sample acquired events*/

    DBObjPtr<SVDGlobalConfigParameters> m_svdGlobalConfig;  /**< SVD Global Configuration payload*/

  };
}

#endif
