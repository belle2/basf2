/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/VariableManager/KLMClusterVariables.h>

#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/KLMCluster.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double klmClusterTiming(const Particle* particle)
    {
      double result = 0.0;

      const KLMCluster* cluster = particle->getKLMCluster();
      if (cluster) {
        result = cluster->getTime();
      }
      return result;
    }

    VARIABLE_GROUP("KLM Cluster");

    REGISTER_VARIABLE("klmClusterTiming", klmClusterTiming,
                      "returns KLMCluster's timing info.");
  }
}
