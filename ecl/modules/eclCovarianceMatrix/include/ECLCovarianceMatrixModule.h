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

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>

namespace Belle2 {

  class EventLevelClusteringInfo;

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

    /** Store object pointer: EventLevelClusteringInfo. */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo;

  public:
    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Name to be used for default option: EventLevelClusteringInfo.*/
    virtual const char* eventLevelClusteringInfoName() const
    { return "EventLevelClusteringInfo" ; }
  }; // end of ECLCovarianceMatrixModule


  /** The very same module but for PureCsI */
  class ECLCovarianceMatrixPureCsIModule : public ECLCovarianceMatrixModule {
  public:
    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** Name to be used for PureCsI option: EventLevelClusteringInfoPureCsI.*/
    virtual const char* eventLevelClusteringInfoName() const override
    { return "EventLevelClusteringInfoPureCsI" ; }
  }; // end of ECLCovarianceMatrixPureCsIModule

} // end of Belle2 namespace
