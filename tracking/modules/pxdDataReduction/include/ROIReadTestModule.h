/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
    FILE* m_pFile; /**< file*/

  };
}
