/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 Belle II Collaboration                               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <string>
#include <boost/format.hpp>

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{
   * \addtogroup tracking_modules
   * \ingroup modules
   * @{ SizeDeterminerModule @} @}
   */

  /** The size determiner module for regions of interest in the PXD.
   */
  class SizeDeterminerModule : public Module {

  public:
    /** Constructor */
    SizeDeterminerModule();

    /** Init the module */
    virtual void initialize();

    /** Show beginRun message */
    virtual void beginRun();

    /** Show progress */
    virtual void event();

  protected:
  };
} // end namespace Belle2

