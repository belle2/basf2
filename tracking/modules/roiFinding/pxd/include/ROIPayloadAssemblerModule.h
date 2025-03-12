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

#include <string>

namespace Belle2 {
  class EventMetaData;
  class ROIid;
  class ROIpayload;

  /** The ROI Payload Assembler Module
   *
   * This module assembles payload for the ROI in the correct format to be sent to the ONSEN
   *
   */

  class ROIPayloadAssemblerModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROIPayloadAssemblerModule();

  private:

    /**
     *Initializes the Module.
     */
    void initialize() override final;

    /** */
    void event() override final;

    StoreArray<ROIid> m_ROIList; /**< the ROIids dataobjects collection */
    StoreObjPtr<EventMetaData> m_eventMetaData; /**< pointer to the dataobject with event number etc */
    StoreObjPtr<ROIpayload> m_roiPayloads; /**< pointer to the ROIpayload dataobject */

    std::string m_ROIListName; /**< name of the ROI list */
    std::string m_ROIpayloadName; /**< name of the payload to be sent to ONSEN */
    unsigned int mSendAllDS; /**< Send all Data (no selection) downscaler; Workaround for missing ONSEN functionality */
    unsigned int mSendROIsDS; /**<  Send ROIs downscaler; Workaround for missing ONSEN functionality */
    unsigned int mCutNrROIs; /**< If Nr of ROIs per DHHID reach this, send out only one full sensor ROI */
    bool mAcceptAll; /**< Accept all event, dont use HLT decision */
    bool mNoRejectFlag; /**< Never reject, just send no ROI */

  };
}
