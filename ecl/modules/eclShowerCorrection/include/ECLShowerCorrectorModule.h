/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower (mainly longitudinal *
 * leakage): corr = (Reconstructed / Truth).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *               Alon Hershenhorn (hersehn@physics.ubc.ca)                *
 *               Suman Koirala (suman@ntu.edu.tw)                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERCORRECTORMODULE_H_
#define ECLSHOWERCORRECTORMODULE_H_

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dbobjects/ECLLeakageCorrection.h>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/DBArray.h>

// OTHER
#include <string>
#include <vector>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLShowerCorrectorModule : public Module {

    public:
      /** Constructor. */
      ECLShowerCorrectorModule();

      /** Destructor. */
      ~ECLShowerCorrectorModule();

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

      //* Prepare correction */
      void prepareLeakageCorrections();

      //* Get correction */
      double getLeakageCorrection(const double theta, const double energy, const double background) const;

      //* Get correction uncertainty */
      double getLeakageCorrectionUncertainty(const double theta, const double energy, const double background) const;

    private:
      DBArray<ECLLeakageCorrection> m_leakageCorrectionArray;  /**< Leakage corrections from DB */

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/

      /** Default name ECLShowers */
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      /** Default name ECLConnectedRegion */
      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }
    }; // end of ECLCRFinderAndSplitterModule


    /** The very same module but for PureCsI */
    class ECLShowerCorrectorPureCsIModule : public ECLShowerCorrectorModule {
    public:

      /** PureCsI name ECLShowersPureCsI */
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      /** PureCsI name ECLConnectedRegionPureCsI */
      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }
    }; // end of ECLCRFinderAndSplitterPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
