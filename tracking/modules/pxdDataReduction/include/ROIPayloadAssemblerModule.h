/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni, Bjoern Spruck          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/dataobjects/ROIid.h>
#include <tracking/dataobjects/ROIpayload.h>
#include <string>

namespace Belle2 {

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

    void event() override final;

    StoreArray<ROIid> m_ROIList;
    StoreObjPtr<EventMetaData> m_eventMetaData;
    StoreObjPtr<ROIpayload> m_roiPayloads;

    std::string m_ROIListName; /**< name of the ROI list */
    std::string m_ROIpayloadName; /**< name of the payload to be sent to ONSEN */
    unsigned int mSendAllDS; /**< Send all Data (no selection) downscaler; Workaround for missing ONSEN functionality */
    unsigned int mSendROIsDS; /**<  Send ROIs downscaler; Workaround for missing ONSEN functionality */
    unsigned int mCutNrROIs; //*< If Nr of ROIs per DHHID reach this, send out only one full sensor ROI */
    bool mAcceptAll; /*< Accept all event, dont use HLT decision */
    bool mNoRejectFlag; /*< Never reject, just send no ROI */

  };
}
