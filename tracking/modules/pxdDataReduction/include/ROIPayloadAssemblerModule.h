/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROI_PAYLOAD_ASSEMBLER_H_
#define ROI_PAYLOAD_ASSEMBLER_H_

#include <framework/core/Module.h>
#include <tracking/dataobjects/ROIrawID.h>
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

    /**
     * Destructor of the module.
     */
    virtual ~ROIPayloadAssemblerModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  protected:

    ROIrawID m_roiraw; /**< 64 bit union containing a single ROI info to be sent to ONSEN*/

    std::string m_ROIListName; /**< name of the ROI list */
    std::string m_ROIpayloadName; /**< name of the payload to be sent to ONSEN */
    bool m_DESYremap; /**< Remap ROI coordinates for DESY TB 2016, WORKAROUND for missing DHH functionality!!!  */
    bool m_DESYROIExtension; /**< Extend ROI coordinates for DESY TB 2016, WORKAROUND for missing DHH functionality!!!  */
    unsigned int mSendAllDS; /**< Send all Data (no selection) downscaler; Workaround for missing ONSEN functionality */
    unsigned int mSendROIsDS; /**<  Send ROIs downscaler; Workaround for missing ONSEN functionality */

    signed int m_triggerNumberShift; /**< ideally = 0. Workaround to find the pxd trigger number in the onsen for Testbeam 16*/
    void DESYremap(unsigned int& tmpRowMin, unsigned int& tmpRowMax, unsigned int& tmpColMin, unsigned int& tmpColMax);

  };
}
#endif
