/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                              *
 *                                                                        *
 **************************************************************************/

#ifndef VXDHITMODULE_H
#define VXDHITMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstruction of the VXD */
  namespace TB {

    /** VXDHitModule: A module to summarize testbeam simulation. */
    class VXDHitModule : public Module {
    public:
      /** Constructor.  */
      VXDHitModule();

      /** Initialize the module and check the parameters */
      virtual void initialize();
      /** Initialize the list of existing PXD Sensors */
      virtual void beginRun();
      /** Digitize one event */
      virtual void event();
      /** Terminate the module */
      virtual void terminate();

    protected:
      // No data members.

    };//end class declaration
  } // end namespace TB
} // end namespace Belle2

#endif // VXDHITMODULE_H
