/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/core/Module.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>

#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/geometry/ECLNeighbours.h>

#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCellIdMapping.h>
#include <ecl/dataobjects/ECLDsp.h>

namespace Belle2 {

  namespace Variable {

    enum PSDVarType {
      onlineEnergy = 1,   // (called energy)
      fractionOfShowerEnergy = 2,
      offlineEnergy = 3, // (twoComponentTotalEnergy)
      diodeEnergy = 4, //twoComponentDiodeEnergy
      hadronEnergy = 5, // twoComponentHadronEnergy
      hadronEnergyFraction = 6,
      digitWeight = 7, //weight
      digitFitType = 8, //twoComponentFitType
      radius = 9, //
      theta = 10,
      cosTheta = 11,
      phi = 12,
    };

    ECLShower* getECLShowerFromParticle(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return nullptr;
      const auto relShowers = cluster->getRelationsWith<ECLShower>();
      if (relShowers.size() == 0) return nullptr;
      // can this relation ever be 1 cluster : many showers?
      if (relShowers.size() != 1) B2ERROR("Cluster to shower relation vector has size " << relShowers.size());

      ECLShower* shower = relShowers.object(0);
      return shower;
    }


    // returns the nm zernike moment (between 10 and 66)
    double getAbsZernikeMomentNM(const Particle* particle,
                                 const std::vector<double>& arguments)
    {
      if (arguments.size() != 2) {
        B2FATAL("Wrong number of arguments, 2 required for meta function absZernikeMoment");
      }
      const long n = std::lround(arguments[0]);
      const long m = std::lround(arguments[1]);

      if ((n < 1) or (n > 6)) {
        B2FATAL("n must be between 1 and 6 for meta function absZernikeMoment");
      }
      if (m > n) {
        B2FATAL("m must be less than or equal to n for meta function absZernikeMoment");
      }

      ECLShower* shower = getECLShowerFromParticle(particle);
      if (!shower) return std::numeric_limits<float>::quiet_NaN();
      return shower->getAbsZernikeMoment(n, m);
    }


    // Large overlap with similar function in ECLCalDigitVariables.
    // TODO - Possibly integrate into ECLCalDigitVariables.
    double getDigitVariable(const Particle* particle, const std::vector<double>& arguments, const PSDVarType varType)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments, 1 required for meta function getDigitVariable");
      }
      const unsigned int digit = std::lround(arguments[0]);

      ECLShower* shower = getECLShowerFromParticle(particle);
      if (!shower) return std::numeric_limits<float>::quiet_NaN();

      auto relatedDigits = shower->getRelationsTo<ECLCalDigit>();

      const std::vector<std::pair<unsigned int, bool>> idxAndQualityList = shower->getListOfCrystalEnergyRankAndQuality();

      // return nan if we ask for the nth crystal when there are less than n in the shower
      if (digit >= idxAndQualityList.size()) return std::numeric_limits<float>::quiet_NaN();

      const auto idx = idxAndQualityList.at(digit).first;
      const auto goodFit = idxAndQualityList.at(digit).second;
      const auto caldigit = relatedDigits.object(idx);
      const auto digitEnergy = caldigit->getEnergy();

      // variables that are / rely on the fit result should only be returned if there actually was a good fit
      // variables like position and online energy can always be returned
      if (!goodFit and ((varType == PSDVarType::hadronEnergy) ||
                        (varType == PSDVarType::diodeEnergy) ||
                        (varType == PSDVarType::offlineEnergy) ||
                        (varType == PSDVarType::hadronEnergyFraction) ||
                        (varType == PSDVarType::digitFitType))) return std::numeric_limits<float>::quiet_NaN();


      if (varType == PSDVarType::hadronEnergy) return caldigit->getTwoComponentHadronEnergy();
      if (varType == PSDVarType::diodeEnergy) return caldigit->getTwoComponentDiodeEnergy();
      if (varType == PSDVarType::offlineEnergy) return caldigit->getTwoComponentTotalEnergy();
      if (varType == PSDVarType::onlineEnergy) return caldigit->getEnergy();
      if (varType == PSDVarType::fractionOfShowerEnergy) return caldigit-> getEnergy() / shower->getEnergy();
      if (varType == PSDVarType::hadronEnergyFraction) return caldigit->getTwoComponentHadronEnergy()  / digitEnergy;
      if (varType == PSDVarType::digitWeight) return relatedDigits.weight(idx);
      if (varType == PSDVarType::digitFitType) return caldigit->getTwoComponentFitType();

      const int cellId = caldigit->getCellId();
      ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();
      B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
      B2Vector3D showerPosition;
      showerPosition.SetMagThetaPhi(shower->getR(), shower->getTheta(), shower->getPhi());

      TVector3 tempP = showerPosition - calDigitPosition;
      if (varType == PSDVarType::radius) return tempP.Mag();
      if (varType == PSDVarType::theta) return tempP.Theta();
      if (varType == PSDVarType::cosTheta) return tempP.CosTheta();
      if (varType == PSDVarType::phi) return tempP.Phi();

      B2FATAL("variable id not found:" << digit << varType);
      return std::numeric_limits<float>::quiet_NaN();
    }

    // returns the hadronEnergy of the ith ECL crystal
    double getHadronEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::hadronEnergy);
    }

    // returns the diodeEnergy of the ith ECL crystal
    double getDiodeEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::diodeEnergy);
    }

    // returns the online energy of the ith ECL crystal
    double getOnlineEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::onlineEnergy);
    }

    // returns the offline energy of the ith ECL crystal
    double getOfflineEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::offlineEnergy);
    }
    // returns the hadron energy fraction of the ith ECL crystal
    double getHadronEnergyFraction(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::hadronEnergyFraction);
    }

    // returns the digit weight of the ith most energetic ECL crystal
    double getDigitWeight(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::digitWeight);
    }

    // returns the digitFitType of the ith most energetic ECL crystal
    double getDigitFitType(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::digitFitType);
    }

    // returns the digitFitType of the ith most energetic ECL crystal
    double getDigitRadius(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::radius);
    }

    // returns the digitFitType of the ith most energetic ECL crystal
    double getDigitTheta(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::theta);
    }

    // returns the digitFitType of the ith most energetic ECL crystal
    double getDigitCosTheta(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::cosTheta);
    }

    // returns the digitFitType of the ith most energetic ECL crystal
    double getDigitPhi(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::phi);
    }

    // returns the digitFitType of the ith most energetic ECL crystal
    double getFractionOfShowerEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::fractionOfShowerEnergy);
    }


    VARIABLE_GROUP("ECL Charged PID Expert Variables (cDST)");

    // Zernike moments
    REGISTER_VARIABLE("absZernikeMoment(n, m)", getAbsZernikeMomentNM,
                      "[eclChargedPIDExpert] the absolute value of zernike moment nm. Requires n <= 6 and m <= n.");

    //ECLCalDigit info
    REGISTER_VARIABLE("hadronEnergy(i)", getHadronEnergy,
                      "[eclChargedPIDExpert] the hadron energy of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("diodeEnergy(i)", getDiodeEnergy,
                      "[eclChargedPIDExpert] the diode energy of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("onlineEnergy(i)", getOnlineEnergy,
                      "[eclChargedPIDExpert] the online energy of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("offlineEnergy(i)", getOfflineEnergy,
                      "[eclChargedPIDExpert] the online energy of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("fractionOfShowerEnergy(i)", getFractionOfShowerEnergy,
                      "[eclChargedPIDExpert] the fraction of total shower energy in the digit ith digit.");
    REGISTER_VARIABLE("hadronEnergyFraction(i)", getHadronEnergyFraction,
                      "[eclChargedPIDExpert] the hadron energy fraction of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("digitWeight(i)", getDigitWeight,
                      "[eclChargedPIDExpert] the digit weight of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("digitFitType(i)", getDigitFitType,
                      "[eclChargedPIDExpert] the digit weight of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("digitRadius(i)", getDigitRadius,
                      "[eclChargedPIDExpert] the magnitude of the vector from the ith most energetic crystal in the shower to the shower center.");
    REGISTER_VARIABLE("digitTheta(i)", getDigitTheta,
                      "[eclChargedPIDExpert] theta of the vector from the ith most energetic crystal in the shower to the shower center.");
    REGISTER_VARIABLE("digitCosTheta(i)", getDigitCosTheta,
                      "[eclChargedPIDExpert] CosTheta of the vector from the ith most energetic crystal in the shower to the shower center.");
    REGISTER_VARIABLE("digitPhi(i)", getDigitPhi,
                      "[eclChargedPIDExpert] phi of the vector from the ith most energetic crystal in the shower to the shower center.");
  }
}
