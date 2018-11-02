/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//#include <ecl/variables/ECLCalDigitVariables.h>

// analysis
#include <analysis/VariableManager/Manager.h>

// framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>

#include <analysis/dataobjects/Particle.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLCellIdMapping.h>
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
      phi = 20,
      theta = 21,
      phiId = 22,
      thetaId = 23,
      cellId = 24
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
      int maxCellId = -1;
      double maxEnergy = -1.;
      const ECLCluster* cluster = particle->getECLCluster();
      if (cluster) {
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
          }
        } else {
          return std::numeric_limits<double>::quiet_NaN();
        }
      }

      return std::numeric_limits<double>::quiet_NaN();
    }

  }

  namespace Variable {

    //! @returns the eclcaldigit energy
    double getECLCalDigitEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::energy, ECLCalDigitVariable::centerType::maxCell};
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

    //! @returns the eclcaldigit two component hadron energy from ext
    double getExtECLCalDigitTwoComponentHadronEnergy(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::twoComponentHadronEnergy, ECLCalDigitVariable::centerType::extCell};
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

    //! @returns the eclcaldigit theta
    double getTheta(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::theta, ECLCalDigitVariable::centerType::maxCell};
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

    //! @returns the eclcaldigit theta id
    double getThetaId(const Particle* particle, const std::vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (cellid and neighbour area size).");
      }

      std::vector<double> parameters {vars[0], vars[1], ECLCalDigitVariable::varType::thetaId, ECLCalDigitVariable::centerType::maxCell};
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


    VARIABLE_GROUP("ECL Calibration");
    REGISTER_VARIABLE("eclcaldigitEnergy(i, j)", getECLCalDigitEnergy,
                      "[calibration] Returns the energy  of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitTime(i, j)", getECLCalDigitTime,
                      "[calibration] Returns the time of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitTimeResoltion(i, j)", getECLCalDigitTimeResolution,
                      "[calibration] Returns the time resolution (dt99) of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentChi2(i, j)", getTwoComponentChi2,
                      "[calibration] Returns the two component fit chi2 of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentTotalEnergy(i, j)", getTwoComponentTotalEnergy,
                      "[calibration] Returns the two component total energy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitTwoComponentHadronEnergy(i, j)", getTwoComponentHadronEnergy,
                      "[calibration] Returns the two component hadron energy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitPhi(i, j)", getPhi,
                      "[calibration] Returns phi of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitTheta(i, j)", getTheta,
                      "[calibration] Returns theta of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitPhiId(i, j)", getPhiId,
                      "[calibration] Returns the phi Id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitThetaId(i, j)", getThetaId,
                      "[calibration] Returns the theta Id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours");
    REGISTER_VARIABLE("eclcaldigitCellId(i, j)", getCellId,
                      "[calibration] Returns the cell id of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours (1-based)");
    REGISTER_VARIABLE("eclcaldigitCenterCellId", getCenterCellId, "[calibration] Returns the center cell id");
    REGISTER_VARIABLE("eclcaldigitCenterCellThetaId", getCenterCellThetaId, "[calibration] Returns the center cell theta id");
    REGISTER_VARIABLE("eclcaldigitCenterCellIndex(i)", getCenterCellIndex,
                      "[calibration] Returns the center cell index (within its 5x5 (j=5) or 7x7 (j=7) neighbours)");

    VARIABLE_GROUP("ECL Calibration (based on extrapolated tracks)");
    REGISTER_VARIABLE("eclcaldigitExtEnergy(i, j)", getExtECLCalDigitEnergy,
                      "[calibration] Returns the energy  of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTime(i, j)", getExtECLCalDigitTime,
                      "[calibration] Returns the time of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentTotalEnergy(i, j)", getExtECLCalDigitTwoComponentTotalEnergy,
                      "[calibration] Returns the TwoComponentTotalEnergy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentHadronEnergy(i, j)", getExtECLCalDigitTwoComponentHadronEnergy,
                      "[calibration] Returns the TwoComponentHadronEnergy of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtTwoComponentChi2(i, j)", getExtECLCalDigitTwoComponentChi2,
                      "[calibration] Returns the TwoComponentchi2 of the i-th caldigit for 5x5 (j=5) or 7x7 (j=7) neighbours for an ext track");
    REGISTER_VARIABLE("eclcaldigitExtCellId", getExtCellId, "[calibration] Returns the extrapolated cell id");
    REGISTER_VARIABLE("eclcaldigitExtCellThetaId", getExtCellThetaId, "[calibration] Returns the ext cell theta id");
    REGISTER_VARIABLE("eclcaldigitExtCenterCellIndex(i)", getExtCenterCellIndex,
                      "[calibration] Returns the center cell index (within its 5x5 (j=5) or 7x7 (j=7) neighbours) for an ext track");

  }

  // Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableECLCalDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableECLCalDigitVariables);
}
