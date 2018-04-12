/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Main reconstruction splitter code for the N2 hypothesis                *
 * (neutral hadron). This code must run after the N1 splitter             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {

  /** Class to perform the shower correction */
  class ECLSplitterN2Module : public Module {

  public:
    /** Constructor. */
    ECLSplitterN2Module();

    /** Destructor. */
    ~ECLSplitterN2Module();

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

    // Position
    std::string m_positionMethod;  /**< Position calculation: lilo or linear */
    double m_liloParameterA; /**< lin-log parameter A */
    double m_liloParameterB; /**< lin-log parameter B */
    double m_liloParameterC; /**< lin-log parameter C */
    std::vector< double > m_liloParameters; /**< lin-log parameters A, B, and C */

    // Background
    //const int c_fullBkgdCount = 280; /**< Number of expected background digits at full background, FIXME: ove to database. */

    /** Store array: ECLCalDigit. */
    StoreArray<ECLCalDigit> m_eclCalDigits;

    /** Store array: ECLConnectedRegion. */
    StoreArray<ECLConnectedRegion> m_eclConnectedRegions;

    /** Store array: ECLLocalMaximum. */
    StoreArray<ECLLocalMaximum> m_eclLocalMaximums;

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** Default name ECLCalDigits */
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** Default name ECLConnectedRegions */
    virtual const char* eclConnectedRegionArrayName() const
    { return "ECLConnectedRegions" ; }

    /** Default name ECLLocalMaximums */
    virtual const char* eclLocalMaximumArrayName() const
    { return "ECLLocalMaximums" ; }

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

  }; // end of ECLSplitterN2Module


  /** The very same module but for PureCsI */
  class ECLSplitterN2PureCsIModule : public ECLSplitterN2Module {
  public:
    /** PureCsI name ECLCalDigitsPureCsI */
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** PureCsI name ECLConnectedRegionsPureCsI */
    virtual const char* eclConnectedRegionArrayName() const override
    { return "ECLConnectedRegionsPureCsI" ; }

    /** PureCsI name ECLLocalMaximumsPureCsI */
    virtual const char* eclLocalMaximumArrayName() const override
    { return "ECLLocalMaximumsPureCsI" ; }

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

  }; // end of ECLSplitterN2PureCsIModule

} // end of Belle2 namespace
