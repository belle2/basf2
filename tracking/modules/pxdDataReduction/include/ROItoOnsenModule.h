/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROI_TO_ONSEN_H_
#define ROI_TO_ONSEN_H_

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/pxdDataReductionClasses/ROIrawID.h>
#include <string>

namespace Belle2 {

  /** The ROI to ONSEN Module
   *
   * this module translate the ROIs into the ONSEN format
   *
   */

  class ROItoOnsenModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROItoOnsenModule();

    /**
     * Destructor of the module.
     */
    virtual ~ROItoOnsenModule();

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

    ROIrawID m_roiraw;

    int m_dummyTriggerNumber;

    std::string m_ROIListName;
    std::string m_ROIpayloadName;

  };
}
#endif
