/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module converts the ecl dataobject(s) in the mdst dataobect(s)    *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLFINALIZERMODULE_H_
#define ECLFINALIZERMODULE_H_

#include <framework/core/Module.h>

namespace Belle2 {

  /** Class to perform the shower correction */
  class ECLFinalizerModule : public Module {

  public:
    /** Constructor. */
    ECLFinalizerModule();

    /** Destructor. */
    ~ECLFinalizerModule();

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
    double m_clusterEnergyCutMin; /**< Min value for the cluster energy cut. */
    double m_clusterTimeCutMaxEnergy; /**< Above this energy, keep all cluster */

  public:
    /** We need names for the data objects to differentiate between PureCsI and default*/

    /** Default name ECLShower */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Default name ECLCluster */
    virtual const char* eclClusterArrayName() const
    { return "ECLClusters"; }

    /** Default name ECLCalDigits */
    virtual const char* eclCalDigitArrayName() const
    {return "ECLCalDigits";}

  }; // end of ECLFinalizerModule


  /** The very same module but for PureCsI */
  class ECLFinalizerPureCsIModule : public ECLFinalizerModule {
  public:

    /** PureCsI name ECLShower */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** PureCsI name ECLCluster */
    virtual const char* eclClusterArrayName() const override
    { return "ECLClustersPureCsI"; }

    /** PureCsI name ECLCalDigits */
    virtual const char* eclCalDigitArrayName() const
    {return "ECLCalDigitsPureCsI";}
  }; // end of ECLFinalizerPureCsIModule

} // end of Belle2 namespace

#endif
