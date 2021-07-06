/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {

  class EventT0;
  class ECLShower;
  class ECLCluster;
  class ECLCalDigit;
  class EventLevelClusteringInfo;

  /** Class to perform the shower correction */
  class ECLFinalizerModule : public Module {

  public:
    /** Constructor. */
    ECLFinalizerModule();

    /** Destructor. */
    ~ECLFinalizerModule();

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
    double m_clusterEnergyCutMin; /**< Min value for the cluster energy cut. */
    double m_clusterTimeCutMaxEnergy; /**< Above this energy, keep all cluster */
    double m_clusterLossyFraction; /**< Maximum allowed fractional difference between nPhotons and neutralHadron number of crystals */

    StoreArray<ECLCalDigit> m_eclCalDigits; /**< ECLCalDigits */
    StoreArray<ECLShower> m_eclShowers; /**< ECLShowers */
    StoreArray<ECLCluster> m_eclClusters; /**< ECLClusters */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo; /**< EventLevelClusteringInfo */
    StoreObjPtr<EventT0> m_eventT0; /**< Event T0 */

    int makeCluster(int, double); /**< Make a cluster from a given shower array index */

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
    virtual const char* eclCalDigitArrayName() const override
    {return "ECLCalDigitsPureCsI";}
  }; // end of ECLFinalizerPureCsIModule

} // end of Belle2 namespace
