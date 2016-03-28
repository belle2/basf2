/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERSHAPEMODULE_H_
#define ECLSHOWERSHAPEMODULE_H_

#include <framework/core/Module.h>
#include <ecl/dataobjects/ECLShower.h>

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

      /** Shower shape variable: Lateral energy. */
      double computeLateralEnergy(const ECLShower&) const;
    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/

      /** Default name ECLShowers */
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      /** Default name ECLCalDigits */
      virtual const char* eclCalDigitArrayName() const
      { return "ECLCalDigits" ; }

      /** Default name ECLConnectedRegions */
      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }

    }; // end of ECLShowerShapeModule


    /** The very same module but for PureCsI */
    class ECLShowerShapePureCsIModule : public ECLShowerShapeModule {
    public:

      /** PureCsI name ECLShowersPureCsI */
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      /** PureCsI name ECLCalDigitsPureCsI */
      virtual const char* eclCalDigitArrayName() const override
      { return "ECLCalDigitsPureCsI" ; }

      /** PureCsI name ECLConnectedRegionsPureCsI */
      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }

    }; // end of ECLShowerShapePureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
