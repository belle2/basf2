/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  class EventLevelClusteringInfo;
  class ECLShower;

  /** Class to perform the shower correction */
  class ECLCovarianceMatrixModule : public Module {

  public:
    /** Constructor. */
    ECLCovarianceMatrixModule();

    /** Destructor. */
    ~ECLCovarianceMatrixModule();

    /** Initialize. */
    virtual void initialize() override;

    /** Begin run. */
    virtual void beginRun() override;

    /** Event. */
    virtual void event() override;

    /** End run. */
    virtual void endRun() override;

    /** Terminate. */
    virtual void terminate() override;

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
