/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        if (clusterShowerRelations.size() == 1) {
          return clusterShowerRelations.object(0)->getNumberOfCrystalsForEnergy();
        } else {
          B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        }
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    //! @returns nominal number of crystals used to calculate energy
    double getShowerNominalNumberOfCrystalsForEnergy(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        if (clusterShowerRelations.size() == 1) {
          return clusterShowerRelations.object(0)->getNominalNumberOfCrystalsForEnergy();
        } else {
          B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        }
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    //! @returns hadron intensity of the shower
    double getShowerHadronIntensity(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        if (clusterShowerRelations.size() == 1) {
          return clusterShowerRelations.object(0)->getShowerHadronIntensity();
        } else {
          B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        }
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    //! @returns number of hadron digits of the shower
    double getShowerNumberOfHadronDigits(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        if (clusterShowerRelations.size() == 1) {
          return clusterShowerRelations.object(0)->getNumberOfHadronDigits();
        } else {
          B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        }
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    // returns the nm zernike moment (between 10 and 55)
    double getAbsZernikeMomentNM(const Particle* particle,
                                 const std::vector<double>& arguments)
    {
      if (arguments.size() != 2) {
        B2FATAL("Wrong number of arguments, 2 required for meta function absZernikeMoment");
      }
      const long n = std::lround(arguments[0]);
      const long m = std::lround(arguments[1]);

      if ((n < 1) or (n > 5)) {
        B2FATAL("n must be between 1 and 5 for meta function absZernikeMoment");
      }
      if (m > n) {
        B2FATAL("m must be less than or equal to n for meta function absZernikeMoment");
      }

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        if (clusterShowerRelations.size() == 1) {
          return clusterShowerRelations.object(0)->getAbsZernikeMoment(n, m);
        } else {
          B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        }
      }
      return std::numeric_limits<float>::quiet_NaN();
    }


    VARIABLE_GROUP("ECL Shower Debugging (cDST)");

    REGISTER_VARIABLE("eclShowerNumberOfCrystalsForEnergy", getShowerNumberOfCrystalsForEnergy,
                      "[debugging] Returns the number of crystals ued to calculate the shower energy (optimized to minimize the energy resolution). This should not be confused with the number of crystals contained in the cluster. ");

    REGISTER_VARIABLE("eclShowerNominalNumberOfCrystalsForEnergy", getShowerNominalNumberOfCrystalsForEnergy,
                      "[debugging] Returns the nominal number of crystals ued to calculate the shower energy (optimized to minimize the energy resolution). This should not be confused with the number of crystals contained in the cluster. ");

    REGISTER_VARIABLE("eclShowerHadronIntensity", getShowerHadronIntensity,
                      "[debugging] Returns the hadron intensity of the shower associated with the particle.");

    REGISTER_VARIABLE("eclShowerNumberOfHadronDigits", getShowerNumberOfHadronDigits,
                      "[debugging] Returns the number of hadron digits of the shower associated with the particle.");

    VARIABLE_GROUP("ECL Shower Variables (cDST)");

    REGISTER_VARIABLE("absZernikeMoment(n, m)", getAbsZernikeMomentNM,
                      "[eclChargedPIDExpert] the absolute value of zernike moment nm. Requires n <= 5 and m <= n.");
  }
}
