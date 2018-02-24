/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates the covariance matrix for n photon showers.     *
 * The matrix depends on the shower region (FWD, Bartel, BWD)             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCOVARIANCEMATRIXMODULE_H_
#define ECLCOVARIANCEMATRIXMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLEventInformation.h>

namespace Belle2 {

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

  private:

    const double m_fullBkgdCount = 183.0; /**< Nominal Background at BGx1.0 (MC12) */

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** Store object pointer: ECLEventInformation. */
    StoreObjPtr<ECLEventInformation> m_eclEventInformation;

  public:
    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Name to be used for default option: ECLEventInformation.*/
    virtual const char* eclEventInformationName() const
    { return "ECLEventInformation" ; }
  }; // end of ECLCovarianceMatrixModule


  /** The very same module but for PureCsI */
  class ECLCovarianceMatrixPureCsIModule : public ECLCovarianceMatrixModule {
  public:
    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** Name to be used for PureCsI option: ECLEventInformationPureCsI.*/
    virtual const char* eclEventInformationName() const override
    { return "ECLEventInformationPureCsI" ; }
  }; // end of ECLCovarianceMatrixPureCsIModule

} // end of Belle2 namespace

#endif
