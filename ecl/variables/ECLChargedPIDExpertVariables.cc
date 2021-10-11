/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: M Hohmann                                                *
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
      hadronEnergy = 1,
      onlineEnergy = 2,
      hadronEnergyFraction = 3,
      digitWeight = 4,
      digitFitType = 5,
      radius = 6,
      theta = 7,
      phi = 8,
      cosTheta = 9,
    };



    ECLShower* getECLShowerFromParticle(const Particle* particle)
    {

      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return nullptr;
      const auto relShowers = cluster->getRelationsWith<ECLShower>();
      if (relShowers.size() == 0) return nullptr;
      B2WARNING("TEST");
      // can this relation ever be 1 cluster : many showers?
      if (relShowers.size() != 1) B2INFO("Rel vector has size " << relShowers.size()); // TEMP

      ECLShower* shower = relShowers.object(0);
      return shower;
    }


    std::vector<std::tuple<double, unsigned int>> getDigitEnergyToSortVector(const ECLShower* shower)
    {

      std::vector<std::tuple<double, unsigned int>> EnergyToSort;
      auto relatedDigits = shower->getRelationsTo<ECLCalDigit>();

      //EnergyToSort vector is used for sorting digits by offline two component energy
      for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

        const auto caldigit = relatedDigits.object(iRel);

        //exclude digits without waveforms
        const double digitChi2 = caldigit->getTwoComponentChi2();
        if (digitChi2 < 0)  continue;

        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();

        //exclude digits digits with poor chi2
        if (digitFitType1 == ECLDsp::poorChi2) continue;

        //exclude digits with diode-crossing fits
        if (digitFitType1 == ECLDsp::photonDiodeCrossing) continue;

        EnergyToSort.emplace_back(caldigit->getTwoComponentTotalEnergy(), iRel);
      }
      return EnergyToSort;
    }


    // returns the nm zernike moment (between 00 and 66)
    double getAbsZernikeMomentNM(const Particle* particle,
                                 const std::vector<double>& arguments)
    {
      if (arguments.size() != 2) {
        B2FATAL("Wrong number of arguments, 2 required for meta function absZernikeMoment");
      }
      const long n = std::lround(arguments[0]);
      const long m = std::lround(arguments[1]);

      ECLShower* shower = getECLShowerFromParticle(particle);
      if (!shower) return std::numeric_limits<float>::quiet_NaN();
      return shower->getAbsZernikeMoment(n, m);
    }

    double getDigitVariable(const Particle* particle, const std::vector<double>& arguments, const PSDVarType varType)
    {
      if (arguments.size() != 1) {
        B2FATAL("Wrong number of arguments, 1 required for meta function getDigitVariable");
      }
      const unsigned int digit = std::lround(arguments[0]);

      ECLShower* shower = getECLShowerFromParticle(particle);
      if (!shower) return std::numeric_limits<float>::quiet_NaN();

      auto relatedDigits = shower->getRelationsTo<ECLCalDigit>();

      std::vector<std::tuple<double, unsigned int>> EnergyToSort = getDigitEnergyToSortVector(shower);
      if (EnergyToSort.size() <= digit) return std::numeric_limits<float>::quiet_NaN();

      //sorting by energy
      std::sort(EnergyToSort.begin(), EnergyToSort.end(), std::greater<>());

      const auto [digitEnergy, next] = EnergyToSort[digit];
      const auto caldigit = relatedDigits.object(next);

      if (varType == PSDVarType::hadronEnergy) return caldigit->getTwoComponentHadronEnergy();
      if (varType == PSDVarType::onlineEnergy) return caldigit->getEnergy();
      if (varType == PSDVarType::hadronEnergyFraction) return caldigit->getTwoComponentHadronEnergy()  / digitEnergy;
      if (varType == PSDVarType::digitWeight) return relatedDigits.weight(next);
      if (varType == PSDVarType::digitFitType) return caldigit->getTwoComponentFitType();

      const int cellId = caldigit->getCellId();
      ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();
      B2Vector3D calDigitPosition = geometry->GetCrystalPos(cellId - 1);
      B2Vector3D showerPosition;
      showerPosition.SetMagThetaPhi(shower->getR(), shower->getTheta(), shower->getPhi());

      TVector3 tempP = showerPosition - calDigitPosition;
      if (varType == PSDVarType::radius) return tempP.Mag();
      if (varType == PSDVarType::theta) return tempP.Theta();
      if (varType == PSDVarType::phi) return tempP.Phi();
      return std::numeric_limits<float>::quiet_NaN();
    }

    // returns the hadronEnergy of the ith ECL crystal
    double getHadronEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::hadronEnergy);
    }

    // returns the online energy of the ith ECL crystal
    double getOnlineEnergy(const Particle* particle, const std::vector<double>& arguments)
    {
      return getDigitVariable(particle, arguments, PSDVarType::onlineEnergy);
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


    VARIABLE_GROUP("ECL Charged PID Expert Variables (cDST)");

    REGISTER_VARIABLE("absZernikeMoment(n, m)", getAbsZernikeMomentNM,
                      "[eclChargedPIDExpert] the absolute value of zernike moment nm. Requires n <= 6 and m <= n.");
    REGISTER_VARIABLE("hadronEnergy(i)", getHadronEnergy,
                      "[eclChargedPIDExpert] the hadron energy of the ith most energetic crystal in the shower.");
    REGISTER_VARIABLE("onlineEnergy(i)", getOnlineEnergy,
                      "[eclChargedPIDExpert] the online energy of the ith most energetic crystal in the shower.");
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

