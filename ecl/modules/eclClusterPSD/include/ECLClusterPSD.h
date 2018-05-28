/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Savino Longo (longos@uvic.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

// STL
#include <vector>

namespace Belle2 {

  class ECLShower;
  class ECLCalDigit;

  /**   This module computes shower variables using pulse shape
       information from offline two component fits.  Using pulse
       shape discrimination, these shower variables can be used
       for particle id. */
  class ECLClusterPSDModule: public Module {

  public:

    /** Constructor.
     */
    ECLClusterPSDModule();

    /** Destructor.
     */
    ~ECLClusterPSDModule();

    /** Initialize variables. */
    virtual void initialize();

    /** begin run.*/
    virtual void beginRun();

    /** event per event.
     */
    virtual void event();

    /** end run. */
    virtual void endRun();

    /** terminate.*/
    virtual void terminate();

    /** ECLCalDigits array name.*/
    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    /** ECLShowers array name.*/
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

  private:
    double m_Chi2Threshold;   /** Two component fit chi square threshold .*/
    double m_CrystalHadronEnergyThreshold; /** hadron component energy threshold to classify as hadron.*/
    StoreArray<ECLCalDigit> m_eclCalDigits;  /** ECLCalDigit's */
    StoreArray<ECLShower> m_eclShowers;  /** ECLShower's */
  };

  /** Class derived from ECL, only difference are the names */
  class ECLClusterPSDPureCsIModule : public ECLClusterPSDModule {
  public:
    /** PureCsI Name of the ECLCalDigitsPureCsI.*/
    virtual const char* eclCalDigitArrayName() const override
    { return "ECLCalDigitsPureCsI" ; }

    /** PureCsI Name of the ECLShowersPureCsI.*/
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

  };

} // end Belle2 namespace
