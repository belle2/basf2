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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCalDigit.h>

// OTHER
#include <vector>

namespace Belle2 {

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

    virtual const char* eclCalDigitArrayName() const
    { return "ECLCalDigits" ; }

    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Name of the ECLEventInformation.*/
    virtual const char* eclEventInformationName() const
    { return "ECLEventInformation" ; }

  private:
    double m_Chi2Threshold;   /** Two component fit chi square threshold .*/
    double m_CrystalHadronEnergyThreshold; /** hadron component energy threshold to classify as hadron.*/
    StoreArray<ECLCalDigit> eclCalDigits;  /** ECLCalDigit's */
    StoreArray<ECLShower> eclShowers;  /** ECLShower's */
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

    /** PureCsI Name of the ECLEventInformationPureCsI.*/
    virtual const char* eclEventInformationName() const override
    { return "ECLEventInformationPureCsI" ; }
  };

} // end Belle2 namespace
