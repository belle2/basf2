/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the calibration for EM shower. This calibration   *
 * corrects for differences between data and MC: corr = data / MC         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERCALIBRATORMODULE_H_
#define ECLSHOWERCALIBRATORMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {

  /** Class to perform the shower correction */
  class ECLShowerCalibratorModule : public Module {

  public:
    /** Constructor. */
    ECLShowerCalibratorModule();

    /** Destructor. */
    ~ECLShowerCalibratorModule();

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

#endif
