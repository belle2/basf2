/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include

#include <analysis/VariableManager/KlIdVariables.h>
#include <analysis/VariableManager/Manager.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <reconstruction/dataobjects/KlId.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double particleKlId(const Particle* particle)
    {
      double result = -999;
      const KlId* klid = particle->getKLMCluster()->getRelatedTo<KlId>();
      if (klid) {
        result = klid->getKlId();
      }
      return result;
    }




    VARIABLE_GROUP("K_L0-ID");
    REGISTER_VARIABLE("KlId", particleKlId, "KlId from KLMcluster classifier.");

  }
}
