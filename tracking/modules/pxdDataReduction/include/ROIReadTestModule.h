/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <stdio.h>

namespace Belle2 {

  /** The ROI to ONSEN Module
   *
   * this module is used to check the payload produced by the ROIPayloadAssembler Module
   *
   */

  class ROIReadTestModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROIReadTestModule();

  private:

    /**
     *Initializes the Module.
     */
    void initialize() override;

    void event() override;

    /**
     * Termination action.
     */
    void terminate() override;

    std::string m_outfileName; /**< produced file name*/
    std::string m_ROIpayloadName; /**< ROI payload name*/
    FILE* m_pFile = nullptr; /**< file handler*/

  };
}
