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

// for crystal geometry
#include <ecl/geometry/ECLGeometryPar.h>
#include <framework/geometry/B2Vector3.h>

// dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>

#include <analysis/dataobjects/Particle.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLCellIdMapping.h>
#include <tracking/dataobjects/ExtHit.h>

#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/geometry/ECLGeometryPar.h>

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
      thetaPointing = 33
    };

    // enum with available center types
    enum centerType {
      maxCell = 0,
      extCell = 1
    };


    //! @returns center cell
    int getCenterCell(const Particle* particle)
    {
      // get maximum cell id for this cluster (ignore weights)
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        int maxCellId = -1;
        double maxEnergy = -1.;

        auto clusterDigitRelations = cluster->getRelationsTo<ECLCalDigit>();
        for (unsigned int ir = 0; ir < clusterDigitRelations.size(); ++ir) {
          const auto calDigit = clusterDigitRelations.object(ir);

          if (calDigit->getEnergy() > maxEnergy) {
            maxEnergy = calDigit->getEnergy();
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

    //! @returns variable requested (expert function, only called from this file)
    double getCalDigitExpertByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {

      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (energy rank, variable id).");
      }

      if (int(std::lround(vars[0])) < 0)  B2FATAL("Index cannot be negative.");

      const unsigned int indexIn = int(std::lround(vars[0]));

      const int varid = int(std::lround(vars[1]));

      //EnergyToSort vector is used for sorting digits by digit energy measured by FPGAs
      std::vector<std::tuple<double, unsigned int>> energyToSort;

      const ECLCluster* cluster = particle->getECLCluster();

      if (cluster) {

        auto relatedDigits = cluster->getRelationsTo<ECLCalDigit>();

        if (indexIn < relatedDigits.size()) {

          for (unsigned int iRel = 0; iRel < relatedDigits.size(); iRel++) {

            const auto caldigit = relatedDigits.object(iRel);

            energyToSort.emplace_back(caldigit->getEnergy(), iRel);

          }

        } else {
          return std::numeric_limits<double>::quiet_NaN();
        }

        std::sort(energyToSort.begin(), energyToSort.end(), std::greater<>());

        const auto [digitEnergy, caldigitIndex] = energyToSort[indexIn];

        const auto caldigitSelected = relatedDigits.object(caldigitIndex);

        // Mapping object for phi & theta
        StoreObjPtr<ECLCellIdMapping> mapping;
        if (!mapping) {
          B2ERROR("Mapping not found, did you forget to run the eclFillCellIdMapping module?");
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
          const auto weight = relatedDigits.weight(caldigitIndex);
          return weight;
        } else if (varid == varType::phi) {
          return mapping->getCellIdToPhi(caldigitSelected->getCellId());
        } else if (varid == varType::theta) {
          return mapping->getCellIdToTheta(caldigitSelected->getCellId());
        } else if (varid == varType::R_geom) {
          return getCellIdMagnitude(caldigitSelected->getCellId());
        } else {
          B2FATAL("variable id not found.");
        }

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
      if (nneighbours != 5 and nneighbours != 7) {
        B2FATAL("Please request 5 or 7 neighbour area.");
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
              auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>(); // should never happen to have more than one shower
              for (unsigned int ir = 0; ir < clusterShowerRelations.size(); ++ir) {
                const auto shower = clusterShowerRelations.object(ir);
                listCellIds = shower->getListOfCrystalsForEnergy();
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
        const TVector3& extHitPosition = edgeExtHit->getPosition();
        const TVector3& trackPointing = edgeExtHit->getMomentum();

        geometry->Mapping(edgeExtHit->getCopyID() - 1);
        const int thetaID = geometry->GetThetaID();
        const int phiID = geometry->GetPhiID();

        const TVector3& crystalCenterPosition = geometry->GetCrystalPos(geometry->GetCellID(thetaID, phiID));
        const TVector3& crystalOrientation = geometry->GetCrystalVec(geometry->GetCellID(thetaID, phiID));
        const TVector3& crystalPositionOnSurface = crystalCenterPosition - (crystalCenterPosition - extHitPosition).Dot(
                                                     crystalOrientation.Unit()) * crystalOrientation.Unit();

        if (varid == varType::phiOffset) {
          return extHitPosition.DeltaPhi(crystalPositionOnSurface);
        } else if (varid == varType::thetaOffset) {
          return extHitPosition.Theta() - crystalPositionOnSurface.Theta();
        } else if (varid == varType::phiPointing) {
          return trackPointing.DeltaPhi(crystalOrientation);
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
        B2FATAL("Need exactly one parameters (energy index).");
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
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentFitType};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 by digit energy rank
    double getTwoComponentChi2ByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentChi2};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component total energy by digit energy rank
    double getTwoComponentTotalEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentTotalEnergy};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component hadron energy by digit energy rank
    double getTwoComponentHadronEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentHadronEnergy};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component diode energy by digit energy rank
    double getTwoComponentDiodeEnergyByEnergyRank(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentDiodeEnergy};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 for photon+hadron fit type by digit energy rank
    double getTwoComponentChi2SavedByEnergyRank_PhotonHadron(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonHadron};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 for photon+hadron + pile-up photon fit type by digit energy rank
    double getTwoComponentChi2SavedByEnergyRank_PileUpPhoton(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentSavedChi2_PileUpPhoton};
      return ECLCalDigitVariable::getCalDigitExpertByEnergyRank(particle, parameters);
    }

    //! @returns the eclcaldigit two component chi2 for photon+diode fit type by digit energy rank
    double getTwoComponentChi2SavedByEnergyRank_PhotonDiode(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameters (energy index).");
      }
      std::vector<double> parameters {vars[0], ECLCalDigitVariable::varType::twoComponentSavedChi2_PhotonDiode};
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

    //! @returns the eclcaldigit energy
    double getECLCalDigitEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::energy, ECLCalDigitVariable::centerType::maxCell};
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
      if (nneighbours != 5 and nneighbours != 7) {
        B2FATAL("Please request 5 or 7 neighbour area.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int centercellid = ECLCalDigitVariable::getCenterCell(particle);
      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();

      std::vector<short int> neighbours;

      if (nneighbours == 5) {
        neighbours = mapping->getCellIdToNeighbour5(centercellid);
      } else if (nneighbours == 7) {
        neighbours = mapping->getCellIdToNeighbour7(centercellid);
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
      if (nneighbours != 5 and nneighbours != 7) {
        B2FATAL("Please request 5 or 7 neighbour area.");
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int centercellid = ECLCalDigitVariable::getExtCell(particle);
      if (centercellid < 0) return std::numeric_limits<double>::quiet_NaN();

      std::vector<short int> neighbours;

      if (nneighbours == 5) {
        neighbours = mapping->getCellIdToNeighbour5(centercellid);
      } else if (nneighbours == 7) {
        neighbours = mapping->getCellIdToNeighbour7(centercellid);
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

    double getClusterECLCalDigitMCEnergy(const Particle* particle)
    {
      // get MCParticle (return if there is none)
      const MCParticle* mcparticle = particle->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return std::numeric_limits<double>::quiet_NaN();

      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
        std::vector<unsigned int> listCellIds;
        auto clusterShowerRelations = cluster->getRelationsWith<ECLShower>(); // should never happen to have more than one shower
        for (unsigned int ir = 0; ir < clusterShowerRelations.size(); ++ir) {
          const auto shower = clusterShowerRelations.object(ir);
          listCellIds = shower->getListOfCrystalsForEnergy();
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
                      "[calibration] Returns the energy  of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTime(i, j)", getECLCalDigitTime,
                      "[calibration] Returns the time of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTimeResolution(i, j)", getECLCalDigitTimeResolution,
                      "[calibration] Returns the time resolution (dt99) of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2(i, j)", getTwoComponentChi2,
                      "[calibration] Returns the two component fit chi2 of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentTotalEnergy(i, j)", getTwoComponentTotalEnergy,
                      "[calibration] Returns the two component total energy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentHadronEnergy(i, j)", getTwoComponentHadronEnergy,
                      "[calibration] Returns the two component hadron energy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitPhi(i, j)", getPhi,
                      "[calibration] Returns phi of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTheta(i, j)", getTheta,
                      "[calibration] Returns theta of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitR(i, j)", getR,
                      "Returns R (from a geometry object) of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitPhiId(i, j)", getPhiId,
                      "[calibration] Returns the phi Id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitThetaId(i, j)", getThetaId,
                      "[calibration] Returns the theta Id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitCellId(i, j)", getCellId,
                      "[calibration] Returns the cell id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours (1-based)",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitUsedForClusterEnergy(i, j)", getUsedForClusterEnergy,
                      " [calibration] Returns the 0 (not used) 1 (used) of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours (1-based)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitCenterCellId", getCenterCellId, "[calibration] Returns the center cell id",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitCenterCellThetaId", getCenterCellThetaId, "[calibration] Returns the center cell theta id",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitCenterCellPhiId", getCenterCellPhiId, "[calibration] Returns the center cell phi id",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitCenterCellCrystalTheta", getCenterCellCrystalTheta,
                      "[calibration] Returns the center cell crystal theta", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitCenterCellCrystalPhi", getCenterCellCrystalPhi,
                      "[calibration] Returns the center cell crystal phi", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitCenterCellIndex(i)", getCenterCellIndex,
                      "[calibration] Returns the center cell index (within its 5x5 (i=5) or 7x7 (i=7) neighbours)", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitMCEnergy(i, j)", getMCEnergy,
                      "[calibration] Returns the true deposited energy of all particles of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours (1-based)",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("clusterNHitsThreshold(i)", getClusterNHitsThreshold,
                      "[calibration] Returns sum of crystal weights sum(w_i) with w_i<=1  associated to this cluster above threshold (in GeV)",
                      Manager::VariableDataType::c_double);

    VARIABLE_GROUP("ECL Calibration (based on extrapolated tracks) (cDST)");
    REGISTER_VARIABLE("eclcaldigitExtEnergy(i, j)", getExtECLCalDigitEnergy,
                      "[calibration] Returns the energy  of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTime(i, j)", getExtECLCalDigitTime,
                      "[calibration] Returns the time of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTimeResolution(i, j)", getExtECLCalDigitTimeResolution,
                      "[calibration] Returns the time resolution (dt99) of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentTotalEnergy(i, j)", getExtECLCalDigitTwoComponentTotalEnergy,
                      "[calibration] Returns the TwoComponentTotalEnergy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentHadronEnergy(i, j)", getExtECLCalDigitTwoComponentHadronEnergy,
                      "[calibration] Returns the TwoComponentHadronEnergy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2(i, j)", getExtECLCalDigitTwoComponentChi2,
                      "[calibration] Returns the TwoComponentchi2 of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtPhi(i, j)", getExtPhi,
                      "[calibration] Returns phi of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an extrapolated track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTheta(i, j)", getExtTheta,
                      "[calibration] Returns theta of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an extrapolated track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtPhiId(i, j)", getExtPhiId,
                      "[calibration] Returns the phi Id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an extrapolated track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtThetaId(i, j)", getExtThetaId,
                      "[calibration] Returns the theta Id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an extrapolated track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtCellId", getExtCellId, "[calibration] Returns the extrapolated cell id",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtCellThetaId", getExtCellThetaId, "[calibration] Returns the ext cell theta id",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtCellPhiId", getExtCellPhiId, "[calibration] Returns the ext cell phi id",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtCellCrystalTheta", getExtCellCrystalTheta, "[calibration] Returns the ext cell crystal theta",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtCellCrystalPhi", getExtCellCrystalPhi, "[calibration] Returns the ext cell crystal phi",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtCenterCellIndex(i)", getExtCenterCellIndex,
                      "[calibration] Returns the center cell index (within its 5x5 (i=5) or 7x7 (i=7) neighbours) for an ext track",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitExtFrontPositionPhiOffset", getExtFrontPositionPhiOffset,
                      "[calibration] Returns the difference in the azimuthal angle (in radians)"
                      "between the position where the track hit the front face of the ECL and the"
                      "center of the struck crystal projected onto the front surface.", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtFrontPositionThetaOffset", getExtFrontPositionThetaOffset,
                      "[calibration] Returns the difference in the polar angle (in radians)"
                      "between the position where the track hit the front face of the ECL and the"
                      "center of the struck crystal projected onto the front surface.", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtFrontPositionPhiPointing", getExtFrontPositionPhiPointing,
                      "[calibration] Returns the difference in the azimuthal angle (in radians)"
                      "between the momentum direction when the track hit the front face of the ECL and the"
                      "orientation of the struck crystal.", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtFrontPositionThetaPointing", getExtFrontPositionThetaPointing,
                      "[calibration] Returns the difference in the polar angle (in radians)"
                      "between the momentum direction when the track hit the front face of the ECL and the"
                      "orientation of the struck crystal.", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitExtTwoComponentFitType(i, j)", getExtECLCalDigitTwoComponentFitType,
                      "[calibration] Returns the TwoComponentFitType of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentDiodeEnergy(i, j)", getExtECLCalDigitTwoComponentDiodeEnergy,
                      "[calibration] Returns the TwoComponentDiodeEnergy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2Saved_PhotonHadron(i, j)", getExtECLCalDigitTwoComponentChi2Saved_PhotonHadron,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonHadron of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2Saved_PileUpPhoton(i, j)", getExtECLCalDigitTwoComponentChi2Saved_PileUpPhoton,
                      "[calibration] Returns the TwoComponentChi2Saved_PileUpPhoton of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2Saved_PhotonDiode(i, j)", getExtECLCalDigitTwoComponentChi2Saved_PhotonDiode,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonDiode of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentFitType(i, j)", getECLCalDigitTwoComponentFitType,
                      "[calibration] Returns the TwoComponentFitType of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentDiodeEnergy(i, j)", getECLCalDigitTwoComponentDiodeEnergy,
                      "[calibration] Returns the TwoComponentDiodeEnergy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2Saved_PhotonHadron(i, j)", getECLCalDigitTwoComponentChi2Saved_PhotonHadron,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonHadron of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2Saved_PileUpPhoton(i, j)", getECLCalDigitTwoComponentChi2Saved_PileUpPhoton,
                      "[calibration] Returns the TwoComponentChi2Saved_PileUpPhoton of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2Saved_PhotonDiode(i, j)", getECLCalDigitTwoComponentChi2Saved_PhotonDiode,
                      "[calibration] Returns the TwoComponentChi2Saved_PhotonDiode of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitEnergyByEnergyRank(i)", getECLCalDigitEnergyByEnergyRank,
                      "[calibration] Returns the caldigit energy of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTimeByEnergyRank(i)", getECLCalDigitTimeByEnergyRank,
                      "[calibration] Returns the caldigit time of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentFitTypeByEnergyRank(i)", getTwoComponentFitTypeByEnergyRank,
                      "[calibration] Returns the offline fit type of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2ByEnergyRank(i)", getTwoComponentChi2ByEnergyRank,
                      "[calibration] Returns the two component chi2 of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentEnergyByEnergyRank(i)", getTwoComponentTotalEnergyByEnergyRank,
                      "[calibration] Returns the two component total energy of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentHadronEnergyByEnergyRank(i)", getTwoComponentHadronEnergyByEnergyRank,
                      "[calibration] Returns the two component fit Hadron Energy of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentDiodeEnergyByEnergyRank(i)", getTwoComponentDiodeEnergyByEnergyRank,
                      "[calibration] Returns the two component fit Diode Energy of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2SavedByEnergyRank_PhotonHadron(i)", getTwoComponentChi2SavedByEnergyRank_PhotonHadron,
                      "[calibration] Returns the chi2 for the photo+hadron fit type of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2SavedByEnergyRank_PileUpPhoton(i)", getTwoComponentChi2SavedByEnergyRank_PileUpPhoton,
                      "[calibration] Returns the chi2 for the photo+hadron+pile-up photon fit type of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2SavedByEnergyRank_PhotonDiode(i)", getTwoComponentChi2SavedByEnergyRank_PhotonDiode,
                      "[calibration] Returns the chi2 for the photo+diode fit type of the i-th highest energy caldigit in the cluster (i>=0)",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitWeightByEnergyRank(i)", getWeightByEnergyRank,
                      "[calibration] Returns the weight of the i-th highest energy caldigit in the cluster (i>=0)", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitCellIdByEnergyRank(i)", getCellIdByEnergyRank,
                      "[calibration] Returns the cell id of the i-th highest energy caldigit in the cluster (i>=0)", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("totalECLCalDigitMCEnergy", getTotalECLCalDigitMCEnergy,
                      "[calibration] Returns total deposited MC energy in all ECLCalDigits for the MC particle", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("clusterECLCalDigitMCEnergy", getClusterECLCalDigitMCEnergy,
                      "[calibration] Returns total deposited MC energy in all ECLCalDigits for the MC particle that are used to calculate the cluster energy",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitPhiByEnergyRank(i)", getPhiByEnergyRank,
                      "Returns phi of the i-th highest energy caldigit in the cluster (i>=0)", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitThetaByEnergyRank(i)", getThetaByEnergyRank,
                      "Returns theta of the i-th highest energy caldigit in the cluster (i>=0)", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("eclcaldigitRByEnergyRank(i)", getRByEnergyRank,
                      "Returns R of the i-th highest energy caldigit in the cluster (i>=0)", Manager::VariableDataType::c_double);
  }

  // Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableECLCalDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableECLCalDigitVariables); /**< register the empty module */
}
