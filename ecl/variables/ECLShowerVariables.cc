/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// analysis
#include <analysis/VariableManager/Manager.h>

// framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <mdst/dataobjects/ECLCluster.h>

#include <analysis/dataobjects/Particle.h>
#include <ecl/dataobjects/ECLShower.h>

using namespace std;

namespace Belle2 {

  namespace Variable {

    //! @returns number of crystals used to calculate energy
    double getShowerNumberOfCrystalsForEnergy(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        double showerNumberOfCrystalsForEnergy = -1.0;

        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        for (unsigned int ir = 0; ir < clusterShowerRelations.size(); ++ir) {
          const auto shower = clusterShowerRelations.object(ir);

          showerNumberOfCrystalsForEnergy = shower->getNumberOfCrystalsForEnergy();
        }

        return showerNumberOfCrystalsForEnergy;
      }

      return std::numeric_limits<float>::quiet_NaN();
    }

    //! @returns nominal number of crystals used to calculate energy
    double getShowerNominalNumberOfCrystalsForEnergy(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        double showerNominalNumberOfCrystalsForEnergy = -1.0;

        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        for (unsigned int ir = 0; ir < clusterShowerRelations.size(); ++ir) {
          const auto shower = clusterShowerRelations.object(ir);

          showerNominalNumberOfCrystalsForEnergy = shower->getNominalNumberOfCrystalsForEnergy();
        }

        return showerNominalNumberOfCrystalsForEnergy;
      }

      return std::numeric_limits<float>::quiet_NaN();
    }


    VARIABLE_GROUP("ECL Shower Debugging (cDST)");

    REGISTER_VARIABLE("eclShowerNumberOfCrystalsForEnergy", getShowerNumberOfCrystalsForEnergy,
                      "[debugging] Returns the number of crystals ued to calculate the shower energy (optimized to minimize the energy resolution). This should not be confused with the number of crystals contained in the cluster. ");
    REGISTER_VARIABLE("eclShowerNominalNumberOfCrystalsForEnergy", getShowerNominalNumberOfCrystalsForEnergy,
                      "[debugging] Returns the nominal number of crystals ued to calculate the shower energy (optimized to minimize the energy resolution). This should not be confused with the number of crystals contained in the cluster. ");

  }
}
