/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* ECL headers. */
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLCellIdMapping.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLGeometryPar.h>

/* Basf2 headers. */
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/Particle.h>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/geometry/B2Vector3.h>
#include <framework/geometry/VectorUtil.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

using namespace std;

namespace Belle2 {

  namespace ECLCalDigitVariable {

    // enum with available data types
    enum varType {
      energy = 1,
      time = 2,
      timeResolution = 3,
      twoComponentChi2 = 10,
      twoComponentTotalEnergy = 11,
      twoComponentHadronEnergy = 12,
      twoComponentDiodeEnergy = 13,
      twoComponentSavedChi2_PhotonHadron = 14,
      twoComponentSavedChi2_PileUpPhoton = 15,
      twoComponentSavedChi2_PhotonDiode = 16,
      twoComponentFitType = 17,
      weight = 18,
      phi = 20,
      theta = 21,
      phiId = 22,
      thetaId = 23,
      cellId = 24,
      mcenergy = 25,
      usedforenergy = 26,
      R_geom = 27, // Requires a geometry environment
      phiOffset = 30,
      thetaOffset = 31,
      phiPointing = 32,
      thetaPointing = 33,
      twoComponentHadronEnergyFraction = 41,
      fractionOfShowerEnergy = 42,
      phiRelativeToShower = 43,
      thetaRelativeToShower = 44,
      cosThetaRelativeToShower = 45,
      rRelativeToShower = 46,
    };

    // enum with available center types
    enum centerType {
      maxCell = 0,
      extCell = 1
    };


    //! @returns center cell with maximum caldigit energy * reconstructed weight
    int getCenterCell(const Particle* particle)
    {
      // get maximum cell id for this cluster (using energy and weights)
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        int maxCellId = -1;
        double maxEnergy = -1.;

        auto clusterDigitRelations = cluster->getRelationsTo<ECLCalDigit>();
        for (unsigned int ir = 0; ir < clusterDigitRelations.size(); ++ir) {
          const auto calDigit = clusterDigitRelations.object(ir);
          const auto weight = clusterDigitRelations.weight(ir);

          if (calDigit->getEnergy()*weight > maxEnergy) {
            maxEnergy = calDigit->getEnergy() * weight;
            maxCellId = calDigit->getCellId();
          }
        }

        return maxCellId;
      }

      return -1;
    }

    //! @returns ext cell
    int getExtCell(const Particle* particle)
    {
      Const::EDetector myDetID = Const::EDetector::ECL;
      Const::ChargedStable hypothesis = Const::pion;
      int pdgCode = abs(hypothesis.getPDGCode());

      const Track* track = particle->getTrack();
      if (track) {
        for (const auto& extHit : track->getRelationsTo<ExtHit>()) {
          if (abs(extHit.getPdgCode()) != pdgCode) continue;
          if ((extHit.getDetectorID() != myDetID)) continue;
          if (extHit.getStatus() != EXT_ENTER) continue;

          int copyid =  extHit.getCopyID();

          if (copyid == -1) continue;
          const int cellid = copyid + 1;
          return cellid;
        }
      }

      return -1;
    }

    //! @returns the position three vector of a cellid
    // This function requires a geometry environment
    double getCellIdMagnitude(int cellid)
    {
      Belle2::ECL::ECLGeometryPar* geom = Belle2::ECL::ECLGeometryPar::Instance();
      Belle2::B2Vector3D position = geom->GetCrystalPos(cellid - 1);
      return position.Mag();
    }

    //! @returns a vector of indices and quality flags, sorted by online energy * weight
    std::vector<std::pair<unsigned int, bool>> calculateListOfCrystalEnergyRankAndQuality(ECLShower* shower)
    {
      std::vector<std::pair<unsigned int, bool>> listOfCrystalEnergyRankAndQuality;
      std::vector<std::tuple<double, unsigned int, bool>> energyToSort;

      RelationVector<ECLCalDigit> relatedDigits = shower->getRelationsTo<ECLCalDigit>();

      //energyToSort vector is used for sorting digits by calibrated energy
      for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

        const auto caldigit = relatedDigits.object(iRel);
        const auto weight = relatedDigits.weight(iRel);
        bool goodFit = true;

        //exclude digits without waveforms
        const double digitChi2 = caldigit->getTwoComponentChi2();
        if (digitChi2 < 0)  goodFit = false;

        ECLDsp::TwoComponentFitType digitFitType1 = caldigit->getTwoComponentFitType();

        //exclude digits with poor chi2
        if (digitFitType1 == ECLDsp::poorChi2) goodFit = false;

        //exclude digits with diode-crossing fits
        if (digitFitType1 == ECLDsp::photonDiodeCrossing)  goodFit = false;

        energyToSort.emplace_back(caldigit->getEnergy()*weight, iRel, goodFit);
      }

      // sort the vector
      std::sort(energyToSort.begin(), energyToSort.end(), std::greater<>());

      for (unsigned int iSorted = 0; iSorted < energyToSort.size(); iSorted++) {
        listOfCrystalEnergyRankAndQuality.push_back(std::make_pair(std::get<1>(energyToSort[iSorted]),
                                                                   std::get<2>(energyToSort[iSorted])));
      }
      return listOfCrystalEnergyRankAndQuality;
    }

    ECLShower* getECLShowerFromParticle(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (!cluster) return nullptr;
      const auto relShowers = cluster->getRelationsWith<ECLShower>();
      if (relShowers.size() == 0) return nullptr;

      if (relShowers.size() == 1) {
        return relShowers.object(0);
      } else {
        B2FATAL("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        return nullptr;
      }
    }

    //! @returns variable requested (expert function, only called from this file)
    double getCalDigitExpertByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {

      if (!((vars.size() == 2) || (vars.size() == 3))) {
        B2FATAL("Need two or three parameters (energy rank, variable id, [onlyGoodQualityPSDFits]).");
      }

      if (int(std::lround(vars[0])) < 0)  B2FATAL("Index cannot be negative.");

      const unsigned int indexIn = int(std::lround(vars[0]));
      const int varid = int(std::lround(vars[1]));

      bool onlyGoodQualityPSDFits = false;
      if (vars.size() == 3) {
        onlyGoodQualityPSDFits = static_cast<bool>(std::lround(vars[2]));
      }

      ECLShower* shower = getECLShowerFromParticle(particle);
      if (!shower) return std::numeric_limits<float>::quiet_NaN();

      // fill the list if it doesn't exist yet.
      if (shower->getListOfCrystalEnergyRankAndQuality().empty()) {
        shower->setListOfCrystalEnergyRankAndQuality(calculateListOfCrystalEnergyRankAndQuality(shower));
      }

      const std::vector<std::pair<unsigned int, bool>> idxAndQualityList = shower->getListOfCrystalEnergyRankAndQuality();

      // return nan if we ask for the nth crystal when there are less than n in the shower
      if (indexIn >= idxAndQualityList.size()) return std::numeric_limits<float>::quiet_NaN();

      auto relatedDigits = shower->getRelationsTo<ECLCalDigit>();
      const auto calDigitIndex = idxAndQualityList.at(indexIn).first;
      const auto goodFit = idxAndQualityList.at(indexIn).second;
      const auto caldigitSelected = relatedDigits.object(calDigitIndex);
      const auto weight = relatedDigits.weight(calDigitIndex);
      const auto digitEnergy = caldigitSelected->getEnergy() * weight;

      // Mapping object for phi & theta
      StoreObjPtr<ECLCellIdMapping> mapping;
      if ((!mapping) and ((varid == varType::phi) or (varid == varType::theta))) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      // veto bad fits for PSD info if requested
      if (onlyGoodQualityPSDFits && (!goodFit) && ((varid == varType::twoComponentChi2) ||
                                                   (varid == varType::twoComponentTotalEnergy) ||
                                                   (varid == varType::twoComponentHadronEnergy) ||
                                                   (varid == varType::twoComponentDiodeEnergy) ||
                                                   (varid == varType::twoComponentFitType) ||
                                                   (varid == varType::twoComponentHadronEnergyFraction)
                                                  )) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (varid == varType::energy) {
        return caldigitSelected->getEnergy();
      } else if (varid == varType::time) {
        return caldigitSelected->getTime();
      } else if (varid == varType::twoComponentChi2) {
        return caldigitSelected->getTwoComponentChi2();
      } else if (varid == varType::twoComponentTotalEnergy) {
        return caldigitSelected->getTwoComponentTotalEnergy();
      } else if (varid == varType::twoComponentHadronEnergy) {
        return caldigitSelected->getTwoComponentHadronEnergy();
      } else if (varid == varType::twoComponentSavedChi2_PhotonHadron) {
        return caldigitSelected->getTwoComponentSavedChi2(ECLDsp::photonHadron);
      } else if (varid == varType::twoComponentSavedChi2_PileUpPhoton) {
        return caldigitSelected->getTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton);
      } else if (varid == varType::twoComponentSavedChi2_PhotonDiode) {
        return caldigitSelected->getTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing);
      } else if (varid == varType::twoComponentDiodeEnergy) {
        return caldigitSelected->getTwoComponentDiodeEnergy();
      } else if (varid == varType::twoComponentFitType) {
        return int(caldigitSelected->getTwoComponentFitType());
      } else if (varid == varType::cellId) {
        return caldigitSelected->getCellId();
      } else if (varid == varType::weight) {
        return weight;
      } else if (varid == varType::phi) {
        return mapping->getCellIdToPhi(caldigitSelected->getCellId());
      } else if (varid == varType::theta) {
        return mapping->getCellIdToTheta(caldigitSelected->getCellId());
      } else if (varid == varType::R_geom) {
        return getCellIdMagnitude(caldigitSelected->getCellId());
      } else if (varid == varType::twoComponentHadronEnergyFraction) {
        if (caldigitSelected-> getTwoComponentTotalEnergy() > 0) {
          return caldigitSelected->getTwoComponentHadronEnergy() / caldigitSelected->getTwoComponentTotalEnergy();
        } else {
          return 0.0;
        }
      } else if (varid == varType::fractionOfShowerEnergy) {
        return digitEnergy / shower->getEnergy();

      } else if ((varid == varType::phiRelativeToShower) ||
                 (varid == varType::thetaRelativeToShower) ||
                 (varid == varType::cosThetaRelativeToShower) ||
                 (varid == varType::rRelativeToShower)) {
        ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();
        B2Vector3D calDigitPosition = geometry->GetCrystalPos(caldigitSelected->getCellId() - 1);
        B2Vector3D showerPosition;
        showerPosition.SetMagThetaPhi(shower->getR(), shower->getTheta(), shower->getPhi());

        ROOT::Math::XYZVector tempP = showerPosition - calDigitPosition;
        if (varid == varType::rRelativeToShower) return tempP.R();
        if (varid == varType::thetaRelativeToShower) return tempP.Theta();
        if (varid == varType::cosThetaRelativeToShower) return tempP.Z() / tempP.R();
        if (varid == varType::phiRelativeToShower) return tempP.Phi();
      } else {
        B2FATAL("variable id not found.");
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    //! @returns variable requested (expert function, only called from this file)
    double getCalDigitExpert(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 4) {
        B2FATAL("Need exactly four parameters (cellid, neighbour area size, variable id, and cluster center (0) or ext (1)).");
      }

      StoreObjPtr<ECLCellIdMapping> mapping;
      const unsigned int posid = int(std::lround(vars[0]));

      const int nneighbours = int(std::lround(vars[1]));
      const int varid = int(std::lround(vars[2]));
      const int extid = int(std::lround(vars[3]));

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      if (nneighbours != 5 and nneighbours != 7 and nneighbours != 9 and nneighbours != 11) {
        B2FATAL("Please request 5, 7, 9 or 11 neighbour area.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      // get maximum cell id for this cluster (ignore weights)
      // TODO: if eclshowers exist we could skip that step.
      int maxCellId = -1;
      if (extid == centerType::extCell) {
        maxCellId = getExtCell(particle);
      } else {
        maxCellId = getCenterCell(particle);
      }

      if (maxCellId < 0) return std::numeric_limits<double>::quiet_NaN();

      // get the requested neighbourid
      int neighbourid = -1;
      std::vector<short int> neighbours;

      if (nneighbours == 5) {
        neighbours = mapping->getCellIdToNeighbour5(maxCellId);
      } else if (nneighbours == 7) {
        neighbours = mapping->getCellIdToNeighbour7(maxCellId);
      } else if (nneighbours == 9) {
        neighbours = mapping->getCellIdToNeighbour9(maxCellId);
      } else if (nneighbours == 11) {
        neighbours = mapping->getCellIdToNeighbour11(maxCellId);
      }

      if (posid < neighbours.size()) {
        neighbourid = neighbours[posid];
      } else {
        B2WARNING("This position id is not contained in the requested neighbours.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      // some variables can be returned even if no ECLCalDigit is present
      if (varid == varType::phi) {
        return mapping->getCellIdToPhi(neighbourid);
      } else if (varid == varType::theta) {
        return mapping->getCellIdToTheta(neighbourid);
      } else if (varid == varType::R_geom) {
        return getCellIdMagnitude(neighbourid);
      } else if (varid == varType::phiId) {
        return mapping->getCellIdToPhiId(neighbourid);
      } else if (varid == varType::thetaId) {
        return mapping->getCellIdToThetaId(neighbourid);
      } else if (varid == varType::cellId) {
        return neighbourid;
      }
      //... and some really need a ECLCalDigit being present
      else {
        const int storearraypos = mapping->getCellIdToStoreArray(neighbourid);
        StoreArray<ECLCalDigit> eclCalDigits;

        if (storearraypos >= 0) {
          if (varid == varType::energy) {
            return eclCalDigits[storearraypos]->getEnergy();
          } else if (varid == varType::weight) {
            const ECLCluster* cluster = particle->getECLCluster();
            if (cluster == nullptr) {return std::numeric_limits<double>::quiet_NaN();}
            double weight = 0.;
            auto relatedDigits = cluster->getRelationsTo<ECLCalDigit>();
            for (unsigned int iDigit = 0; iDigit < relatedDigits.size(); iDigit++) {
              const auto caldigit = relatedDigits.object(iDigit);
              const int digitCellID = caldigit->getCellId();
              if (digitCellID == neighbourid) {
                weight = relatedDigits.weight(iDigit);
                break;
              }
            }
            return weight;

          } else if (varid == varType::time) {
            return eclCalDigits[storearraypos]->getTime();
          } else if (varid == varType::timeResolution) {
            return eclCalDigits[storearraypos]->getTimeResolution();
          } else if (varid == varType::twoComponentChi2) {
            return eclCalDigits[storearraypos]->getTwoComponentChi2();
          } else if (varid == varType::twoComponentTotalEnergy) {
            return eclCalDigits[storearraypos]->getTwoComponentTotalEnergy();
          } else if (varid == varType::twoComponentHadronEnergy) {
            return eclCalDigits[storearraypos]->getTwoComponentHadronEnergy();
          } else if (varid == varType::twoComponentSavedChi2_PhotonHadron) {
            return eclCalDigits[storearraypos]->getTwoComponentSavedChi2(ECLDsp::photonHadron);
          } else if (varid == varType::twoComponentSavedChi2_PileUpPhoton) {
            return eclCalDigits[storearraypos]->getTwoComponentSavedChi2(ECLDsp::photonHadronBackgroundPhoton);
          } else if (varid == varType::twoComponentSavedChi2_PhotonDiode) {
            return eclCalDigits[storearraypos]->getTwoComponentSavedChi2(ECLDsp::photonDiodeCrossing);
          } else if (varid == varType::twoComponentDiodeEnergy) {
            return eclCalDigits[storearraypos]->getTwoComponentDiodeEnergy();
          } else if (varid == varType::twoComponentFitType) {
            return int(eclCalDigits[storearraypos]->getTwoComponentFitType());
          } else if (varid == varType::mcenergy) {
            // loop over all related MCParticles
            auto digitMCRelations = eclCalDigits[storearraypos]->getRelationsTo<MCParticle>();
            double edep = 0.0;
            for (unsigned int i = 0; i < digitMCRelations.size(); ++i) {
              edep += digitMCRelations.weight(i);
            }
            return edep;
          } else if (varid == varType::usedforenergy) {
            const ECLCluster* cluster = particle->getECLCluster();
            if (cluster) {

              unsigned int cellid = eclCalDigits[storearraypos]->getCellId();
              std::vector<unsigned int> listCellIds;

              auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();

              if (clusterShowerRelations.size() == 1) {
                listCellIds = clusterShowerRelations.object(0)->getListOfCrystalsForEnergy();
              } else {
                B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
              }

              if (std::find(listCellIds.begin(), listCellIds.end(), cellid) != listCellIds.end()) { //find is faster than count
                return 1;
              }

              return 0;
            }
            return std::numeric_limits<double>::quiet_NaN();

          }
        } else {
          return std::numeric_limits<double>::quiet_NaN();
        }
      }

      return std::numeric_limits<double>::quiet_NaN();
    }

    double getExtCellExpert(const Particle* particle, int varid, bool front)
    {
      ECL::ECLGeometryPar* geometry = ECL::ECLGeometryPar::Instance();
      if (!geometry) {
        B2ERROR("Geometry not found!");
        return std::numeric_limits<double>::quiet_NaN();
      }
      const Track* track = particle->getTrack();
      if (track) {
        ExtHit* edgeExtHit = nullptr;
        if (front) {
          for (const auto& extHit : track->getRelationsTo<ExtHit>()) {
            if (extHit.getDetectorID() != Const::EDetector::ECL) continue;
            if (extHit.getStatus() != EXT_ENTER) continue;
            int crystalID = extHit.getCopyID() - 1;
            if (crystalID == -1) continue;
            edgeExtHit = new ExtHit(extHit);
            break;
          }
        } else {
          auto extHits = track->getRelationsTo<ExtHit>();
          for (unsigned int iextHit(extHits.size() - 1); iextHit > 0; --iextHit) {
            const auto extHit = extHits[iextHit];
            if (extHit->getDetectorID() != Const::EDetector::ECL) continue;
            if (extHit->getStatus() != EXT_EXIT) continue;
            int crystalID = extHit->getCopyID() - 1;
            if (crystalID == -1) break;
            edgeExtHit = new ExtHit(*extHit);
            break;
          }
        }

        if (!edgeExtHit) return std::numeric_limits<double>::quiet_NaN();
        const ROOT::Math::XYZVector& extHitPosition = edgeExtHit->getPosition();
        const ROOT::Math::XYZVector& trackPointing = edgeExtHit->getMomentum();

        geometry->Mapping(edgeExtHit->getCopyID() - 1);
        const int thetaID = geometry->GetThetaID();
        const int phiID = geometry->GetPhiID();
        const int cellID = geometry->GetCellID(thetaID, phiID);

        const ROOT::Math::XYZVector& crystalCenterPosition =
          geometry->GetCrystalPos(cellID);
        const ROOT::Math::XYZVector& crystalOrientation =
          geometry->GetCrystalVec(cellID);
        const ROOT::Math::XYZVector& crystalPositionOnSurface =
          crystalCenterPosition -
          (crystalCenterPosition - extHitPosition).Dot(
            crystalOrientation.Unit()) * crystalOrientation.Unit();
        if (varid == varType::phiOffset) {
          return VectorUtil::phiMinusPiPi(extHitPosition.Phi() - crystalPositionOnSurface.Phi());
        } else if (varid == varType::thetaOffset) {
          return extHitPosition.Theta() - crystalPositionOnSurface.Theta();
        } else if (varid == varType::phiPointing) {
          return VectorUtil::phiMinusPiPi(trackPointing.Phi() - crystalOrientation.Phi());
        } else if (varid == varType::thetaPointing) {
          return trackPointing.Theta() - crystalOrientation.Theta();
        }
      }

      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  namespace Variable {

    //! @returns the eclcaldigit energy by digit energy rank
    double getECLCalDigitEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {

      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::energy};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit Time by digit energy rank
    double getECLCalDigitTimeByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::time};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit MCEnergy by digit energy rank
    double getECLCalDigitMCEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::mcenergy};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit cell id by digit energy rank
    double getCellIdByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::cellId};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit fit type by digit energy rank
    double getTwoComponentFitTypeByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentFitType, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 by digit energy rank
    double getTwoComponentChi2ByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentChi2, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component total energy by digit energy rank
    double getTwoComponentTotalEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentTotalEnergy, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component hadron energy by digit energy rank
    double getTwoComponentHadronEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentHadronEnergy, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component hadron energy fraction by digit energy rank
    double getTwoComponentHadronEnergyFractionByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentHadronEnergyFraction, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component diode energy by digit energy rank
    double getTwoComponentDiodeEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentDiodeEnergy, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 for photon+hadron fit type by digit energy rank
    double getTwoComponentChi2SavedByEnergyRank_PhotonHadron(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonHadron, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 for photon+hadron + pile-up photon fit type by digit energy rank
    double getTwoComponentChi2SavedByEnergyRank_PileUpPhoton(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentSavedChi2_PileUpPhoton, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 for photon+diode fit type by digit energy rank
    double getTwoComponentChi2SavedByEnergyRank_PhotonDiode(const Particle* particle, const std::vector<double>& vars)
    {
      if (!((vars.size() == 1) | (vars.size() == 2))) {
        B2FATAL("Need one or two parameters (energy index, [onlyGoodQualityPSDFits]).");
      }
      double onlyGoodQualityPSDFits = 0.0;
      if (vars.size() == 2) {onlyGoodQualityPSDFits = std::lround(vars[1]);}

      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonDiode, onlyGoodQualityPSDFits};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit weight by digit energy rank
    double getWeightByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::weight};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit fraction of Shower energy by digit energy rank
    double getECLCalDigitFractionOfShowerEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::fractionOfShowerEnergy};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns phi of the vector joining the eclcaldigit to the shower center by digit energy rank
    double getECLCalDigitPhiRelativeToShowerByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::phiRelativeToShower};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns theta of the vector joining the eclcaldigit to the shower center by digit energy rank
    double getECLCalDigitThetaRelativeToShowerByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::thetaRelativeToShower};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns cos(theta) of the vector joining the eclcaldigit to the shower center by digit energy rank
    double getECLCalDigitCosThetaRelativeToShowerByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::cosThetaRelativeToShower};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns radius of the vector joining the eclcaldigit to the shower center by digit energy rank
    double getECLCalDigitRadiusRelativeToShowerByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::rRelativeToShower};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit energy
    double getECLCalDigitEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::energy, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }


    //! @returns the eclcaldigit weight
    double getECLCalDigitWeight(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::weight, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the MC deposited energy
    double getMCEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::mcenergy, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }


    //! @returns the eclcaldigit energy from ext
    double getExtECLCalDigitEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::energy, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }


    //! @returns the eclcaldigit time
    double getECLCalDigitTime(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::time, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit time from ext
    double getExtECLCalDigitTime(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::time, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit time from ext
    double getExtECLCalDigitTimeResolution(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::timeResolution, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit time resolution
    double getECLCalDigitTimeResolution(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::timeResolution, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2
    double getTwoComponentChi2(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentChi2, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 from ext
    double getExtECLCalDigitTwoComponentChi2(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentChi2, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component total energy
    double getTwoComponentTotalEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentTotalEnergy, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component total energy from ext
    double getExtECLCalDigitTwoComponentTotalEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentTotalEnergy, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component hadron energy
    double getTwoComponentHadronEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentHadronEnergy, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the 1 if the eclcladigit was used in the energy calculation
    double getUsedForClusterEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::usedforenergy, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component hadron energy from ext
    double getExtECLCalDigitTwoComponentHadronEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentHadronEnergy, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component Diode energy
    double getECLCalDigitTwoComponentDiodeEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentDiodeEnergy, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component Diode energy from ext
    double getExtECLCalDigitTwoComponentDiodeEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentDiodeEnergy, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component Fit Type
    double getECLCalDigitTwoComponentFitType(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentFitType, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component Fit Type from ext
    double getExtECLCalDigitTwoComponentFitType(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentFitType, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component saved chi2 for PhotonHadron fit
    double getECLCalDigitTwoComponentChi2Saved_PhotonHadron(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonHadron, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component saved chi2 for PhotonHadron fit from ext
    double getExtECLCalDigitTwoComponentChi2Saved_PhotonHadron(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonHadron, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component saved chi2 for PileUpPhoton fit
    double getECLCalDigitTwoComponentChi2Saved_PileUpPhoton(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentSavedChi2_PileUpPhoton, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component saved chi2 for PileUpPhoton fit from ext
    double getExtECLCalDigitTwoComponentChi2Saved_PileUpPhoton(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentSavedChi2_PileUpPhoton, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component saved chi2 for PhotonDiode fit
    double getECLCalDigitTwoComponentChi2Saved_PhotonDiode(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonDiode, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit two component saved chi2 for PhotonDiode fit from ext
    double getExtECLCalDigitTwoComponentChi2Saved_PhotonDiode(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonDiode, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit phi
    double getPhi(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::phi, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit phi from ext
    double getExtPhi(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::phi, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit theta
    double getTheta(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::theta, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit theta from ext
    double getExtTheta(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::theta, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit phi id
    double getPhiId(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::phiId, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit phi id from ext
    double getExtPhiId(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::phiId, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit theta id
    double getThetaId(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::thetaId, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit theta id from ext
    double getExtThetaId(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::thetaId, ECLCalDigitVariable::centerType::extCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit cell id
    double getCellId(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::cellId, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit center cell id
    double getCenterCellId(const Particle* particle)
    {
      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);

      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return centercellid;
    }

    //! @returns the eclcaldigit center cell crystal theta
    double getCenterCellCrystalTheta(const Particle* particle)
    {
      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToTheta(centercellid);
    }

    //! @returns the eclcaldigit center cell crystal phi
    double getCenterCellCrystalPhi(const Particle* particle)
    {
      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToPhi(centercellid);
    }

    //! @returns the eclcaldigit ext cell id
    double getExtCellId(const Particle* particle)
    {
      const int extcellid = ECLCalDigitVariable::getExtCell(particle);

      if (extcellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return extcellid;
    }

    //! @returns the eclcaldigit center cell theta id
    double getCenterCellThetaId(const Particle* particle)
    {
      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToThetaId(centercellid);
    }

    //! @returns the eclcaldigit center cell phi id
    double getCenterCellPhiId(const Particle* particle)
    {
      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToPhiId(centercellid);
    }

    //! @returns the eclcaldigit ext cell theta id
    double getExtCellThetaId(const Particle* particle)
    {
      const int extcellid = ECLCalDigitVariable::getExtCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (extcellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToThetaId(extcellid);
    }

    //! @returns the eclcaldigit ext cell phi id
    double getExtCellPhiId(const Particle* particle)
    {
      const int extcellid = ECLCalDigitVariable::getExtCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (extcellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToPhiId(extcellid);
    }

    //! @returns the eclcaldigit ext cell crystal theta
    double getExtCellCrystalTheta(const Particle* particle)
    {
      const int extcellid = ECLCalDigitVariable::getExtCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (extcellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToTheta(extcellid);
    }

    //! @returns the eclcaldigit ext cell crystal phi
    double getExtCellCrystalPhi(const Particle* particle)
    {
      const int extcellid = ECLCalDigitVariable::getExtCell(particle);
      StoreObjPtr<ECLCellIdMapping> mapping;

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (extcellid < 0) return std::numeric_limits<double>::quiet_NaN();
      return mapping->getCellIdToPhi(extcellid);
    }

    //! @returns the eclcaldigit center cell index
    double getCenterCellIndex(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (neighbour area size).");
      }

      StoreObjPtr<ECLCellIdMapping> mapping;
      const int nneighbours = int(std::lround(vars[0]));

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      if (nneighbours != 5 and nneighbours != 7 and nneighbours != 9 and nneighbours != 11) {
        B2FATAL("Please request 5, 7, 9 or 11 neighbour area.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);
      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();

      std::vector<short int> neighbours;

      if (nneighbours == 5) {
        neighbours = mapping->getCellIdToNeighbour5(centercellid);
      } else if (nneighbours == 7) {
        neighbours = mapping->getCellIdToNeighbour7(centercellid);
      } else if (nneighbours == 9) {
        neighbours = mapping->getCellIdToNeighbour9(centercellid);
      } else if (nneighbours == 11) {
        neighbours = mapping->getCellIdToNeighbour11(centercellid);
      }

      for (unsigned int idx = 0; idx < neighbours.size(); idx++) {
        if (neighbours[idx] == centercellid) return idx;
      }

      return std::numeric_limits<double>::quiet_NaN();
    }


    //! @returns the eclcaldigit center cell index for an ext track
    double getExtCenterCellIndex(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (neighbour area size).");
      }

      StoreObjPtr<ECLCellIdMapping> mapping;
      const int nneighbours = int(std::lround(vars[0]));

      if (!mapping) {
        B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
        return std::numeric_limits<double>::quiet_NaN();
      }
      if (nneighbours != 5 and nneighbours != 7 and nneighbours != 9 and nneighbours != 11) {
        B2FATAL("Please request 5, 7, 9 or 11 neighbour area.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int centercellid = ECLCalDigitVariable::getExtCell(particle);
      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();

      std::vector<short int> neighbours;

      if (nneighbours == 5) {
        neighbours = mapping->getCellIdToNeighbour5(centercellid);
      } else if (nneighbours == 7) {
        neighbours = mapping->getCellIdToNeighbour7(centercellid);
      } else if (nneighbours == 9) {
        neighbours = mapping->getCellIdToNeighbour9(centercellid);
      } else if (nneighbours == 11) {
        neighbours = mapping->getCellIdToNeighbour11(centercellid);
      }

      for (unsigned int idx = 0; idx < neighbours.size(); idx++) {
        if (neighbours[idx] == centercellid) return idx;
      }

      return std::numeric_limits<double>::quiet_NaN();
    }

    double getTotalECLCalDigitMCEnergy(const Particle* particle)
    {
      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      double sum = 0.0;
      auto mcDigitRelations = mcparticle->getRelationsWith<ECLCalDigit>();
      for (unsigned int ir = 0; ir < mcDigitRelations.size(); ++ir) {
        sum += mcDigitRelations.weight(ir);
      }

      return sum;
    }

    double getClusterTotalECLCalDigitMCEnergy(const Particle* particle)
    {
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster == nullptr) {return std::numeric_limits<double>::quiet_NaN();}
      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr) {return std::numeric_limits<double>::quiet_NaN();}

      double sum = 0.0;
      auto relatedDigits = cluster->getRelationsTo<ECLCalDigit>();
      for (unsigned int iDigit = 0; iDigit < relatedDigits.size(); iDigit++) {
        const auto caldigit = relatedDigits.object(iDigit);
        auto mcDigitRelations = caldigit->getRelationsTo<MCParticle>();
        for (unsigned int i = 0; i < mcDigitRelations.size(); i++) {
          const MCParticle* digitmcparticle = mcDigitRelations.object(i);
          if (digitmcparticle == mcparticle) {
            sum += mcDigitRelations.weight(i);
          }
        }
      }
      return sum;

    }

    double getClusterECLCalDigitMCEnergy(const Particle* particle)
    {
      // get MCParticle (return if there is none)
      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        std::vector<unsigned int> listCellIds;
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>();
        if (clusterShowerRelations.size() == 1) {
          listCellIds = clusterShowerRelations.object(0)->getListOfCrystalsForEnergy();
        } else {
          B2ERROR("Somehow found more than 1 ECLShower matched to the ECLCluster. This should not be possible!");
        }

        double sum = 0.0;
        auto clusterDigitRelations = mcparticle->getRelationsWith<ECLCalDigit>();
        for (unsigned int ir = 0; ir < clusterDigitRelations.size(); ++ir) {

          // check if this digit is in the current cluster
          unsigned int cellid = clusterDigitRelations.object(ir)->getCellId();
          if (std::find(listCellIds.begin(), listCellIds.end(), cellid) != listCellIds.end()) { //find is faster than count
            sum += clusterDigitRelations.weight(ir);
          }
        }

        return sum;
      }
      return std::numeric_limits<float>::quiet_NaN();
    }

    double getExtFrontPositionPhiOffset(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::phiOffset, true);
    }

    double getExtFrontPositionThetaOffset(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::thetaOffset, true);
    }

    double getExtFrontPositionPhiPointing(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::phiPointing, true);
    }

    double getExtFrontPositionThetaPointing(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::thetaPointing, true);
    }

    double getExtBackPositionPhiOffset(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::phiOffset, false);
    }

    double getExtBackPositionThetaOffset(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::thetaOffset, false);
    }

    double getExtBackPositionPhiPointing(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::phiPointing, false);
    }

    double getExtBackPositionThetaPointing(const Particle* particle)
    {
      return ECLCalDigitVariable::getExtCellExpert(particle, ECLCalDigitVariable::varType::thetaPointing, false);
    }

    //! @returns the eclcaldigit Phi by digit energy rank
    double getPhiByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::phi};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit Theta digit energy rank
    double getThetaByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::theta};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit R
    double getR(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::R_geom, ECLCalDigitVariable::centerType::maxCell};
      return ECLCalDigitVariable::getCalDigitExpert(particle, parameters);
    }

    //! @returns the eclcaldigit R by digit energy rank
    double getRByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::R_geom};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    double getClusterNHitsThreshold(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (energy threshold in GeV).");
      }
      const double threshold = vars[0];

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        double nhits = 0.;

        auto clusterDigitRelations = cluster->getRelationsTo<ECLCalDigit>();
        for (unsigned int ir = 0; ir < clusterDigitRelations.size(); ++ir) {
          const auto calDigit = clusterDigitRelations.object(ir);
          const auto weight = clusterDigitRelations.weight(ir);

          // take the unweighted eclcaldigit energy for this check (closer to real hardware threshold)
          if (calDigit->getEnergy() > threshold) {
            nhits += weight;
          }
        }

        return nhits;
      }
      return std::numeric_limits<float>::quiet_NaN();
    }


    VARIABLE_GROUP("ECL Calibration (cDST)");
    REGISTER_VARIABLE("eclcaldigitEnergy(i, j)", getECLCalDigitEnergy,
                      "[calibration] Returns the energy  of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitWeight(i, j)", getECLCalDigitWeight,
                      "[calibration] Returns the weight of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTime(i, j)", getECLCalDigitTime,
                      "[calibration] Returns the time of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTimeResolution(i, j)", getECLCalDigitTimeResolution,
                      "[calibration] Returns the time resolution (dt99) of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2(i, j)", getTwoComponentChi2,
                      "[calibration] Returns the two component fit chi2 of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentTotalEnergy(i, j)", getTwoComponentTotalEnergy,
                      "[calibration] Returns the two component total energy of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentHadronEnergy(i, j)", getTwoComponentHadronEnergy,
                      "[calibration] Returns the two component hadron energy of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitPhi(i, j)", getPhi,
                      "[calibration] Returns phi of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTheta(i, j)", getTheta,
                      "[calibration] Returns theta of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitR(i, j)", getR,
                      "Returns R (from a geometry object) of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitPhiId(i, j)", getPhiId,
                      "[calibration] Returns the phi Id of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitThetaId(i, j)", getThetaId,
                      "[calibration] Returns the theta Id of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitCellId(i, j)", getCellId,
                      "[calibration] Returns the cell id of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours (1-based)");
    REGISTER_VARIABLE("eclcaldigitUsedForClusterEnergy(i, j)", getUsedForClusterEnergy,
                      " [calibration] Returns the 0 (not used) 1 (used) of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours (1-based)");

    REGISTER_VARIABLE("eclcaldigitCenterCellId", getCenterCellId, "[calibration] Returns the center cell id");
    REGISTER_VARIABLE("eclcaldigitCenterCellThetaId", getCenterCellThetaId, "[calibration] Returns the center cell theta id");
    REGISTER_VARIABLE("eclcaldigitCenterCellPhiId", getCenterCellPhiId, "[calibration] Returns the center cell phi id");
    REGISTER_VARIABLE("eclcaldigitCenterCellCrystalTheta", getCenterCellCrystalTheta,
                      "[calibration] Returns the center cell crystal theta");
    REGISTER_VARIABLE("eclcaldigitCenterCellCrystalPhi", getCenterCellCrystalPhi,
                      "[calibration] Returns the center cell crystal phi");
    REGISTER_VARIABLE("eclcaldigitCenterCellIndex(i)", getCenterCellIndex,
                      "[calibration] Returns the center cell index (within its 5x5 (i=5), 7x7 (i=7), 9x9 (i=9) or 11x11 (i=11) neighbours neighbours)");
    REGISTER_VARIABLE("eclcaldigitMCEnergy(i, j)", getMCEnergy,
                      "[calibration] Returns the true deposited energy of all particles of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours (1-based)");
    REGISTER_VARIABLE("clusterNHitsThreshold(i)", getClusterNHitsThreshold,
                      "[calibration] Returns sum of crystal weights sum(w_i) with w_i<=1  associated to this cluster above threshold (in GeV)");

    VARIABLE_GROUP("ECL Calibration (based on extrapolated tracks) (cDST)");
    REGISTER_VARIABLE("eclcaldigitExtEnergy(i, j)", getExtECLCalDigitEnergy,
                      "[calibration] Returns the energy  of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTime(i, j)", getExtECLCalDigitTime,
                      "[calibration] Returns the time of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTimeResolution(i, j)", getExtECLCalDigitTimeResolution,
                      "[calibration] Returns the time resolution (dt99) of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentTotalEnergy(i, j)", getExtECLCalDigitTwoComponentTotalEnergy,
                      "[calibration] Returns the TwoComponentTotalEnergy of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentHadronEnergy(i, j)", getExtECLCalDigitTwoComponentHadronEnergy,
                      "[calibration] Returns the TwoComponentHadronEnergy of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2(i, j)", getExtECLCalDigitTwoComponentChi2,
                      "[calibration] Returns the TwoComponentchi2 of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtPhi(i, j)", getExtPhi,
                      "[calibration] Returns phi of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an extrapolated track");
    REGISTER_VARIABLE("eclcaldigitExtTheta(i, j)", getExtTheta,
                      "[calibration] Returns theta of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an extrapolated track");
    REGISTER_VARIABLE("eclcaldigitExtPhiId(i, j)", getExtPhiId,
                      "[calibration] Returns the phi Id of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11)) neighbours for an extrapolated track");
    REGISTER_VARIABLE("eclcaldigitExtThetaId(i, j)", getExtThetaId,
                      "[calibration] Returns the theta Id of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an extrapolated track");
    REGISTER_VARIABLE("eclcaldigitExtCellId", getExtCellId, "[calibration] Returns the extrapolated cell id");
    REGISTER_VARIABLE("eclcaldigitExtCellThetaId", getExtCellThetaId, "[calibration] Returns the ext cell theta id");
    REGISTER_VARIABLE("eclcaldigitExtCellPhiId", getExtCellPhiId, "[calibration] Returns the ext cell phi id");
    REGISTER_VARIABLE("eclcaldigitExtCellCrystalTheta", getExtCellCrystalTheta, "[calibration] Returns the ext cell crystal theta");
    REGISTER_VARIABLE("eclcaldigitExtCellCrystalPhi", getExtCellCrystalPhi, "[calibration] Returns the ext cell crystal phi");
    REGISTER_VARIABLE("eclcaldigitExtCenterCellIndex(i)", getExtCenterCellIndex,
                      "[calibration] Returns the center cell index (within its 5x5 (i=5), 7x7 (i=7), 9x9 (i=9) or 11x11 (i=11) neighbours) for an ext track");

    REGISTER_VARIABLE("eclcaldigitExtFrontPositionPhiOffset", getExtFrontPositionPhiOffset,
                      "[calibration] Returns the difference in the azimuthal angle (in radians)"
                      "between the position where the track hit the front face of the ECL and the"
                      "center of the struck crystal projected onto the front surface.");
    REGISTER_VARIABLE("eclcaldigitExtFrontPositionThetaOffset", getExtFrontPositionThetaOffset,
                      "[calibration] Returns the difference in the polar angle (in radians)"
                      "between the position where the track hit the front face of the ECL and the"
                      "center of the struck crystal projected onto the front surface.");
    REGISTER_VARIABLE("eclcaldigitExtFrontPositionPhiPointing", getExtFrontPositionPhiPointing,
                      "[calibration] Returns the difference in the azimuthal angle (in radians)"
                      "between the momentum direction when the track hit the front face of the ECL and the"
                      "orientation of the struck crystal.");
    REGISTER_VARIABLE("eclcaldigitExtFrontPositionThetaPointing", getExtFrontPositionThetaPointing,
                      "[calibration] Returns the difference in the polar angle (in radians)"
                      "between the momentum direction when the track hit the front face of the ECL and the"
                      "orientation of the struck crystal.");

    REGISTER_VARIABLE("eclcaldigitExtTwoComponentFitType(i, j)", getExtECLCalDigitTwoComponentFitType,
                      "[calibration] Returns the TwoComponentFitType of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentDiodeEnergy(i, j)", getExtECLCalDigitTwoComponentDiodeEnergy,
                      "[calibration] Returns the TwoComponentDiodeEnergy of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2Saved_PhotonHadron(i, j)", getExtECLCalDigitTwoComponentChi2Saved_PhotonHadron,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonHadron of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2Saved_PileUpPhoton(i, j)", getExtECLCalDigitTwoComponentChi2Saved_PileUpPhoton,
                      "[calibration] Returns the TwoComponentChi2Saved_PileUpPhoton of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2Saved_PhotonDiode(i, j)", getExtECLCalDigitTwoComponentChi2Saved_PhotonDiode,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonDiode of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours for an ext track");

    REGISTER_VARIABLE("eclcaldigitTwoComponentFitType(i, j)", getECLCalDigitTwoComponentFitType,
                      "[calibration] Returns the TwoComponentFitType of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentDiodeEnergy(i, j)", getECLCalDigitTwoComponentDiodeEnergy,
                      "[calibration] Returns the TwoComponentDiodeEnergy of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2Saved_PhotonHadron(i, j)", getECLCalDigitTwoComponentChi2Saved_PhotonHadron,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonHadron of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2Saved_PileUpPhoton(i, j)", getECLCalDigitTwoComponentChi2Saved_PileUpPhoton,
                      "[calibration] Returns the TwoComponentChi2Saved_PileUpPhoton of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2Saved_PhotonDiode(i, j)", getECLCalDigitTwoComponentChi2Saved_PhotonDiode,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonDiode of the i-th caldigit for 5x5 (j=5), 7x7 (j=7), 9x9 (j=9) or 11x11 (j=11) neighbours");

    REGISTER_VARIABLE("eclcaldigitEnergyByEnergyRank(i)", getECLCalDigitEnergyByEnergyRank,
                      "[calibration/eclChargedPIDExpert] Returns the caldigit energy of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTimeByEnergyRank(i)", getECLCalDigitTimeByEnergyRank,
                      "[calibration] Returns the caldigit time of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTwoComponentFitTypeByEnergyRank(i[, b])", getTwoComponentFitTypeByEnergyRank,
                      "[calibration/eclChargedPIDExpert] Returns the offline fit type of the i-th highest energy caldigit in the cluster (i>=0). If b is set to 1.0 only caldigits considered to have good quality PSD fits return PSD information.");

    REGISTER_VARIABLE("eclcaldigitMCEnergyByEnergyRank(i)", getECLCalDigitMCEnergyByEnergyRank,
                      "[calibration] Returns the caldigit MC Energy of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2ByEnergyRank(i[, b])", getTwoComponentChi2ByEnergyRank,
                      "[calibration/eclChargedPIDExpert] Returns the two component chi2 of the i-th highest energy caldigit in the cluster (i>=0). If b is set to 1.0 only caldigits considered to have good quality PSD fits return PSD information.");

    REGISTER_VARIABLE("eclcaldigitTwoComponentEnergyByEnergyRank(i)", getTwoComponentTotalEnergyByEnergyRank,
                      "[calibration] Returns the two component total energy of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTwoComponentHadronEnergyByEnergyRank(i[, b])", getTwoComponentHadronEnergyByEnergyRank,
                      "[calibration/eclChargedPIDExpert] Returns the two component fit Hadron Energy of the i-th highest energy caldigit in the cluster (i>=0). If b is set to 1.0 only caldigits considered to have good quality PSD fits return PSD information.");

    REGISTER_VARIABLE("eclcaldigitTwoComponentDiodeEnergyByEnergyRank(i[, b])", getTwoComponentDiodeEnergyByEnergyRank,
                      "[calibration/eclChargedPIDExpert] Returns the two component fit Diode Energy of the i-th highest energy caldigit in the cluster (i>=0). If b is set to 1.0 only caldigits considered to have good quality PSD fits return PSD information.");

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2SavedByEnergyRank_PhotonHadron(i)", getTwoComponentChi2SavedByEnergyRank_PhotonHadron,
                      "[calibration] Returns the chi2 for the photo+hadron fit type of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2SavedByEnergyRank_PileUpPhoton(i)", getTwoComponentChi2SavedByEnergyRank_PileUpPhoton,
                      "[calibration] Returns the chi2 for the photo+hadron+pile-up photon fit type of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2SavedByEnergyRank_PhotonDiode(i)", getTwoComponentChi2SavedByEnergyRank_PhotonDiode,
                      "[calibration] Returns the chi2 for the photo+diode fit type of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitWeightByEnergyRank(i)", getWeightByEnergyRank,
                      "[calibration/eclChargedPIDExpert] Returns the weight of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitCellIdByEnergyRank(i)", getCellIdByEnergyRank,
                      "[calibration] Returns the cell id of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("totalECLCalDigitMCEnergy", getTotalECLCalDigitMCEnergy,
                      "[calibration] Returns total deposited MC energy in all ECLCalDigits for the MC particle");

    REGISTER_VARIABLE("clusterTotalECLCalDigitMCEnergy", getClusterTotalECLCalDigitMCEnergy,
                      "[calibration] Returns total MC energy deposited in ECLCalDigits in the cluster by the related MC particle");

    REGISTER_VARIABLE("clusterECLCalDigitMCEnergy", getClusterECLCalDigitMCEnergy,
                      "[calibration] Returns total deposited MC energy in all ECLCalDigits for the MC particle that are used to calculate the cluster energy");

    REGISTER_VARIABLE("eclcaldigitPhiByEnergyRank(i)", getPhiByEnergyRank,
                      "Returns phi of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitThetaByEnergyRank(i)", getThetaByEnergyRank,
                      "Returns theta of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitRByEnergyRank(i)", getRByEnergyRank,
                      "Returns R of the i-th highest energy caldigit in the cluster (i>=0)");

    REGISTER_VARIABLE("eclcaldigitTwoComponentHadronEnergyFractionByEnergyRank(i[, b])",
                      getTwoComponentHadronEnergyFractionByEnergyRank,
                      "[eclChargedPIDExpert] Returns the hadron energy fraction of the i-th highest energy caldigit in the cluster (i>=0). If b is set to 1.0 only caldigits considered to have good quality PSD fits return PSD information.");
    REGISTER_VARIABLE("eclcaldigitFractionOfTotalShowerEnergyByEnergyRank(i)", getECLCalDigitFractionOfShowerEnergyByEnergyRank,
                      "[eclChargedPIDExpert] Returns the fraction of the total Shower energy in the i-th highest energy caldigit in the Shower (i>=0). Assumes a photon hypothesis.");
    REGISTER_VARIABLE("eclcaldigitPhiRelativeToShowerByEnergyRank(i)", getECLCalDigitPhiRelativeToShowerByEnergyRank,
                      "[eclChargedPIDExpert] Returns phi of the vector joining the i-th highest energy caldigit in the Shower (i>=0) to the Shower center.");
    REGISTER_VARIABLE("eclcaldigitThetaRelativeToShowerByEnergyRank(i)", getECLCalDigitThetaRelativeToShowerByEnergyRank,
                      "[eclChargedPIDExpert] Returns theta of the vector joining the i-th highest energy caldigit in the Shower (i>=0) to the Shower center.");
    REGISTER_VARIABLE("eclcaldigitCosThetaRelativeToShowerByEnergyRank(i)", getECLCalDigitCosThetaRelativeToShowerByEnergyRank,
                      "[eclChargedPIDExpert] Returns cos(theta) of the vector joining the i-th highest energy caldigit in the Shower (i>=0) to the Shower center.");
    REGISTER_VARIABLE("eclcaldigitRadiusRelativeToShowerByEnergyRank(i)", getECLCalDigitRadiusRelativeToShowerByEnergyRank,
                      "[eclChargedPIDExpert] Returns the magnitude of the vector joining the i-th highest energy caldigit in the Shower (i>=0) to the Shower center.");
  }
//   // Create an empty module which allows basf2 to easily find the library and load it from the steering file
//   class EnableECLCalDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
//   REG_MODULE(EnableECLCalDigitVariables); /**< register the empty module */
}
