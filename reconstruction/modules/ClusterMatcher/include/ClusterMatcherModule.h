
/***************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 *************************************************************************/
#ifndef ClusterMatcherModule_H
#define ClusterMatcherModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/ECLCluster.h>


namespace Belle2 {

  /**
  Match KLM Clusters to close ECL Clusters.
    */
  class ClusterMatcherModule : public Module {

  public:

    /** Constructor */
    ClusterMatcherModule();

    /** Destructor */
    virtual ~ClusterMatcherModule();

    /** init */
    virtual void initialize();

    /** process event */
    virtual void event();



  protected:

  private:


    void matchClusterInCone(const ECLCluster& eclcluster, float coneInRad);


  }; // end class
} // end namespace Belle2

#endif
