/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the calibration for EM shower. This calibration   *
 * corrects for differences between data and MC: corr = data / MC         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//Framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  class ECLShower;
  class ECLConnectedRegion;

  /** Class to perform the shower correction */
  class ECLShowerCalibratorModule : public Module {

  public:
    /** Constructor. */
    ECLShowerCalibratorModule();

    /** Destructor. */
    ~ECLShowerCalibratorModule();

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
    /** StoreArray ECLShower */
    StoreArray<ECLShower> m_eclShowers;
    /** StoreArray ECLConnectedRegion */
    StoreArray<ECLConnectedRegion> m_eclCRs;

  public:
    /** We need names for the data objects to differentiate between PureCsI and default*/

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Default name ECLConnectedRegion */
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

  };


  /** The very same module but for PureCsI */
  class ECLShowerCalibratorPureCsIModule : public ECLShowerCalibratorModule {
  public:

    /** PureCsI name ECLShowers */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** PureCsI name ECLConnectedRegion */
    virtual const char* eclConnectedRegionArrayName() const override
    { return "ECLConnectedRegionsPureCsI" ; }

  };
} // end of Belle2 namespace
