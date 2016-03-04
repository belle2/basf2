/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERSHAPEMODULE_H_
#define ECLSHOWERSHAPEMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLShowerShapeModule : public Module {

    public:
      /** Constructor. */
      ECLShowerShapeModule();

      /** Destructor. */
      ~ECLShowerShapeModule();

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

      virtual const char* eclCalDigitArrayName() const
      { return "ECLCalDigits" ; }

      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }
    }; // end of ECLCRFinderAndSplitterModule


    /** The very same module but for PureCsI */
    class ECLShowerShapePureCsIModule : public ECLShowerShapeModule {
    public:
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      virtual const char* eclCalDigitArrayName() const override
      { return "ECLCalDigitsPureCsI" ; }

      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }
    }; // end of ECLCRFinderAndSplitterPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
