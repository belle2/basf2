/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROI_READ_TEST_H_
#define ROI_READ_TEST_H_

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

    /**
     * Destructor of the module.
     */
    virtual ~ROIReadTestModule();

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
    void terminate() override;

  protected:

    std::string m_outfileName; /**< produced file name*/
    std::string m_ROIpayloadName; /**< ROI payload name*/
    FILE* m_pFile; /**< file*/

  };
}
#endif
