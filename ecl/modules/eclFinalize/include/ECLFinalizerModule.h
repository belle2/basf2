/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module converts the ecl dataobject(s) in the mdst dataobect(s)    *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLFINALIZERMODULE_H_
#define ECLFINALIZERMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLFinalizerModule : public Module {

    public:
      /** Constructor. */
      ECLFinalizerModule();

      /** Destructor. */
      ~ECLFinalizerModule();

      /** Initialize. */
      virtual void initialize();

      /** Begin run. */
      virtual void beginRun();

      /** Event. */
      virtual void event();

      /** End run. */
      virtual void endRun();

      /** Terminate. */
      virtual void terminate();
    private:

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }
      virtual const char* eclClusterArrayName() const
      { return "ECLClusters"; }

    }; // end of ECLFinalizerModule


    /** The very same module but for PureCsI */
    class ECLFinalizerPureCsIModule : public ECLFinalizerModule {
    public:
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }
      virtual const char* eclClusterArrayName() const override
      { return "ECLClustersPureCsI"; }

    }; // end of ECLFinalizerPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
