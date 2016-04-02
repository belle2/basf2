/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates the covariance matrix for a shower.             *
 * The matrix will depend on the shower region (FWD, Bartel, BWD) and     *
 * possibly on the hypothesis.                                            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCOVARIANCEMATRIXMODULE_H_
#define ECLCOVARIANCEMATRIXMODULE_H_

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLShower.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLCovarianceMatrixModule : public Module {

    public:
      /** Constructor. */
      ECLCovarianceMatrixModule();

      /** Destructor. */
      ~ECLCovarianceMatrixModule();

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

      StoreArray<ECLShower> m_eclShowers;
    private:

    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }
    }; // end of ECLCovarianceMatrixModule


    /** The very same module but for PureCsI */
    class ECLCovarianceMatrixPureCsIModule : public ECLCovarianceMatrixModule {
    public:
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }
    }; // end of ECLCovarianceMatrixPureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
