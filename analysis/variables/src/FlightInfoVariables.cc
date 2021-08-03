/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/variables/FlightInfoVariables.h>
#include <framework/logging/Logger.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <framework/utilities/Conversion.h>
#include <boost/algorithm/string.hpp>
#include <mdst/dataobjects/MCParticle.h>
#include <vector>
#include <TMatrixFSym.h>

namespace Belle2 {
  class Particle;

  namespace Variable {

    //Function to check if a beam constrained rave fit has been performed
    bool hasRAVEBeamConstrainedProductionVertex(const Particle* particle)
    {
      bool hasRAVEProdVertex = true;
      std::vector<std::string> directions = {"x", "y", "z"};
      for (auto ielement : directions) {
        std::string prodVertPositionElement = boost::str(boost::format("prodVert%s") % boost::to_upper_copy(ielement));
        hasRAVEProdVertex &= particle -> hasExtraInfo(prodVertPositionElement);
        for (auto jelement : directions) {
          std::string prodVertCovarianceElement = boost::str(boost::format("prodVertS%s%s") % ielement % jelement);
          hasRAVEProdVertex &= particle -> hasExtraInfo(prodVertCovarianceElement);
        }
      }
      return hasRAVEProdVertex;
    }

    // Helper function for flight distance and its uncertainty (provided as it is)
    inline double getFlightInfoBtw(const Particle* particle, const Particle* daughter, double& outErr, const std::string& mode,
                                   const bool motherToGranddaughter = false)
    {
      if (!particle || !daughter) {
        outErr = std::numeric_limits<float>::quiet_NaN();
        return std::numeric_limits<float>::quiet_NaN();
      }
      // check if the particle source is a composite particle
      if (!(particle->getParticleSource() == Particle::EParticleSourceObject::c_Composite) ||
          !(daughter->getParticleSource() == Particle::EParticleSourceObject::c_Composite)) {
        // check if the particle source is a V0
        if (!(particle->getParticleSource() == Particle::EParticleSourceObject::c_V0) ||
            !(daughter->getParticleSource() == Particle::EParticleSourceObject::c_V0)) {
          B2WARNING("Attempting to calculate flight " << mode << " for neither composite particle nor V0");
          outErr = std::numeric_limits<float>::quiet_NaN();
          return std::numeric_limits<float>::quiet_NaN();
        }
      }
      if (!(mode == "distance") && !(mode == "time")) {
        B2WARNING("FlightInfo helper function called with mode '" << mode
                  << "'. Only 'distance' and 'time' are available.");
        outErr = std::numeric_limits<float>::quiet_NaN();
        return std::numeric_limits<float>::quiet_NaN();
      }
      // get TreeFitter values if they exist.
      // Bypass this in case the variables are requested for the granddaughter with respect to the mother as
      // TreeFitter will return the values of the granddaughter with respect to the daughter
      if (!motherToGranddaughter) {
        if (mode == "distance" &&
            daughter->hasExtraInfo("decayLength") &&
            daughter->hasExtraInfo("decayLengthErr")) {
          outErr = daughter -> getExtraInfo("decayLengthErr");
          return daughter -> getExtraInfo("decayLength");
        }
        if (mode == "time" &&
            daughter->hasExtraInfo("lifeTime") &&
            daughter->hasExtraInfo("lifeTimeErr")) {
          outErr = daughter -> getExtraInfo("lifeTimeErr");
          return daughter -> getExtraInfo("lifeTime");
        }
      }


      double mumvtxX = particle->getX();
      double mumvtxY = particle->getY();
      double mumvtxZ = particle->getZ();
      if (particle == daughter) {
        if (hasRAVEBeamConstrainedProductionVertex(particle)) {
          mumvtxX = particle->getExtraInfo("prodVertX");
          mumvtxY = particle->getExtraInfo("prodVertY");
          mumvtxZ = particle->getExtraInfo("prodVertZ");
        } else {
          //if no production vertex assume the particle originated at the ip
          static DBObjPtr<BeamSpot> beamSpotDB;
          mumvtxX = (beamSpotDB->getIPPosition()).X();
          mumvtxY = (beamSpotDB->getIPPosition()).Y();
          mumvtxZ = (beamSpotDB->getIPPosition()).Z();
        }
      }
      //daughter vertex
      double vtxX =  daughter->getX();
      double vtxY =  daughter->getY();
      double vtxZ =  daughter->getZ();
      // daughter MOMENTUM
      double p = daughter->getP();
      double pX = daughter->getPx();
      double pY = daughter->getPy();
      double pZ = daughter->getPz();

      //versor of the daughter momentum
      double nX = pX / p;
      double nY = pY / p;
      double nZ = pZ / p;
      //mother vertex
      double lX = vtxX - mumvtxX;
      double lY = vtxY - mumvtxY;
      double lZ = vtxZ - mumvtxZ;

      //flight distance
      double fD      = lX * nX + lY * nY + lZ * nZ;
      //flight time
      double fT = daughter->getPDGMass() / Const::speedOfLight * fD / p;

      //covariance matrix of momentum and vertex for the Dz
      //ORDER = px,py,pz,E,x,y,z
      TMatrixFSym dauCov = daughter->getMomentumVertexErrorMatrix();
      TMatrixFSym mumCov = particle->getVertexErrorMatrix();   //order: x,y,z
      if (particle == daughter) {
        if (hasRAVEBeamConstrainedProductionVertex(particle)) {
          std::vector<std::string> directions = {"x", "y", "z"};
          for (unsigned int i = 0; i < directions.size(); i++) {
            for (unsigned int j = 0; j < directions.size(); j++) {
              mumCov[i][j] = particle->getExtraInfo(boost::str(boost::format("prodVertS%s%s") % directions[i] % directions[j]));
            }
          }
        } else {
          static DBObjPtr<BeamSpot> beamSpotDB;
          mumCov = beamSpotDB->getCovVertex();
        }
      }
      //compute total covariance matrix
      //ORDER = px dau, py dau, pz dau, E dau, x dau, y dau, z dau, x mum, y mum, z mum

      TMatrixFSym Cov(10);
      for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
          if (i < 7 && j < 7)
            Cov[i][j] = dauCov[i][j];
          else if (i > 6 && j > 6)
            Cov[i][j] = mumCov[i - 7][j - 7];
          else
            Cov[i][j] = 0;

      if (mode == "distance") {
        TMatrixF deriv(10, 1);
        deriv[0][0] = (lX - nX * fD) / p;   //px Daughter
        deriv[1][0] = (lY - nY * fD) / p;   //py Daughter
        deriv[2][0] = (lZ - nZ * fD) / p;   //pz Daughter
        deriv[3][0] = 0; //E Daughter
        deriv[4][0] = nX;   //vtxX Daughter
        deriv[5][0] = nY;   //vtxY Daughter
        deriv[6][0] = nZ;   //vtxZ Daughter
        deriv[7][0] = - nX;   //vtxX Mother
        deriv[8][0] = - nY;   //vtxY Mother
        deriv[9][0] = - nZ;   //vtxZ Mother


        TMatrixF tmp(10, 1);
        tmp.Mult(Cov, deriv);

        TMatrixF result(1, 1);
        result.Mult(deriv.T(), tmp);

        outErr = sqrt(result[0][0]);
        return fD;
      }
      if (mode == "time") {
        TMatrixF deriv(10, 1);
        deriv[0][0] = (daughter->getPDGMass() / Const::speedOfLight * lX - 2 * pX * fT) / p / p; //px Daughter
        deriv[1][0] = (daughter->getPDGMass() / Const::speedOfLight * lY - 2 * pY * fT) / p / p; //py Daughter
        deriv[2][0] = (daughter->getPDGMass() / Const::speedOfLight * lZ - 2 * pZ * fT) / p / p; //pz Daughter
        deriv[3][0] = 0; //E Daughter
        deriv[4][0] = daughter->getPDGMass() / Const::speedOfLight * pX / p / p; //vtxX Daughter
        deriv[5][0] = daughter->getPDGMass() / Const::speedOfLight * pY / p / p; //vtxY Daughter
        deriv[6][0] = daughter->getPDGMass() / Const::speedOfLight * pZ / p / p; //vtxZ Daughter
        deriv[7][0] = - daughter->getPDGMass() / Const::speedOfLight * pX / p / p; //vtxX Mother
        deriv[8][0] = - daughter->getPDGMass() / Const::speedOfLight * pY / p / p; //vtxY Mother
        deriv[9][0] = - daughter->getPDGMass() / Const::speedOfLight * pZ / p / p; //vtxZ Mother


        TMatrixF tmp(10, 1);
        tmp.Mult(Cov, deriv);

        TMatrixF result(1, 1);
        result.Mult(deriv.T(), tmp);
        outErr = sqrt(result[0][0]);
        return fT;
      }
      outErr = std::numeric_limits<float>::quiet_NaN();
      return std::numeric_limits<float>::quiet_NaN();
    }

    // Helper function for MC flight time and distance
    inline double getMCFlightInfoBtw(const MCParticle* mcparticle, const std::string& mode)
    {

      if (mcparticle == nullptr)
        return std::numeric_limits<float>::quiet_NaN();


      if (mode == "distance") {
        TVector3 deltaVtx = mcparticle->getDecayVertex() - mcparticle->getProductionVertex();
        double distance = deltaVtx.Mag();
        if (distance < 0)
          B2WARNING("Negative true flight distance, it's forbidden -> something went wrong.");

        return distance;
      }

      if (mode == "time") {
        double lifetime = mcparticle->getLifetime();
        double mass = mcparticle->getMass();
        double time = std::numeric_limits<float>::quiet_NaN();
        if (mass == 0)
          B2WARNING("you are asking for the proper time of a massless particle which is not allowed, returning -99.");
        else {
          double energy = mcparticle->getEnergy();
          time = lifetime / energy * mass;
        }

        if (time < 0)
          B2WARNING("Negative true proper time, it's forbidden -> something went wrong.");

        return time;

      }
      B2WARNING("MCFlightInfo helper function called with mode '" << mode
                << "'. Only 'distance' and 'time' are available.");
      return std::numeric_limits<float>::quiet_NaN();

    }

    double flightDistance(const Particle* part)
    {
      double flightDistanceError = std::numeric_limits<float>::quiet_NaN();
      return getFlightInfoBtw(part, part, flightDistanceError, "distance");
    }

    double flightTime(const Particle* part)
    {
      double flightTimeError = std::numeric_limits<float>::quiet_NaN();
      return getFlightInfoBtw(part, part, flightTimeError, "time");
    }

    double flightDistanceErr(const Particle* part)
    {
      double flightDistanceError = std::numeric_limits<float>::quiet_NaN();
      getFlightInfoBtw(part, part, flightDistanceError, "distance");
      return flightDistanceError;
    }

    double flightTimeErr(const Particle* part)
    {
      double flightTimeError = std::numeric_limits<float>::quiet_NaN();
      getFlightInfoBtw(part, part, flightTimeError, "time");
      return flightTimeError;
    }

    inline double getVertexDistance(const Particle* particle, const Particle* daughter, double& vertexDistanceErr,
                                    bool prodVertIsIP = false)
    {
      if (!particle || !daughter) {
        vertexDistanceErr = std::numeric_limits<float>::quiet_NaN();
        return std::numeric_limits<float>::quiet_NaN();
      }

      // production vertex
      double prodVtxX = particle->getX();
      double prodVtxY = particle->getY();
      double prodVtxZ = particle->getZ();
      if (particle == daughter || prodVertIsIP) {
        if (particle->hasExtraInfo("prodVertX")) prodVtxX = particle->getExtraInfo("prodVertX");
        if (particle->hasExtraInfo("prodVertY")) prodVtxY = particle->getExtraInfo("prodVertY");
        if (particle->hasExtraInfo("prodVertZ")) prodVtxZ = particle->getExtraInfo("prodVertZ");
      }

      // decay vertex
      double vtxX =  daughter->getX();
      double vtxY =  daughter->getY();
      double vtxZ =  daughter->getZ();

      // difference between vertices
      double lX = vtxX - prodVtxX;
      double lY = vtxY - prodVtxY;
      double lZ = vtxZ - prodVtxZ;

      // vertex distance
      double lD = sqrt(lX * lX + lY * lY + lZ * lZ);

      // covariance matrix of both vertices
      TMatrixFSym decCov = daughter->getVertexErrorMatrix();
      TMatrixFSym prodCov = particle->getVertexErrorMatrix();
      if (particle == daughter || prodVertIsIP) {
        if (particle->hasExtraInfo("prodVertSxx")) prodCov[0][0] = particle->getExtraInfo("prodVertSxx");
        if (particle->hasExtraInfo("prodVertSxy")) prodCov[0][1] = particle->getExtraInfo("prodVertSxy");
        if (particle->hasExtraInfo("prodVertSxz")) prodCov[0][2] = particle->getExtraInfo("prodVertSxz");
        if (particle->hasExtraInfo("prodVertSyx")) prodCov[1][0] = particle->getExtraInfo("prodVertSyx");
        if (particle->hasExtraInfo("prodVertSyy")) prodCov[1][1] = particle->getExtraInfo("prodVertSyy");
        if (particle->hasExtraInfo("prodVertSyz")) prodCov[1][2] = particle->getExtraInfo("prodVertSyz");
        if (particle->hasExtraInfo("prodVertSzx")) prodCov[2][0] = particle->getExtraInfo("prodVertSzx");
        if (particle->hasExtraInfo("prodVertSzy")) prodCov[2][1] = particle->getExtraInfo("prodVertSzy");
        if (particle->hasExtraInfo("prodVertSzz")) prodCov[2][2] = particle->getExtraInfo("prodVertSzz");
      }

      // compute total covariance matrix
      // ORDER = prod x, prod y, prod z, decay x, decay y, decay z

      TMatrixFSym Cov(6);
      for (int i = 0; i < 6; i++)
        for (int j = 0; j < 6; j++)
          if (i < 3 && j < 3)
            Cov[i][j] = prodCov[i][j];
          else if (i > 2 && j > 2)
            Cov[i][j] = decCov[i - 3][j - 3];
          else
            Cov[i][j] = 0;

      TMatrixF deriv(6, 1);
      deriv[0][0] = - lX / lD; // prodVtxX
      deriv[1][0] = - lY / lD; // prodVtxY
      deriv[2][0] = - lZ / lD; // prodVtxZ
      deriv[3][0] =   lX / lD; // vtxX
      deriv[4][0] =   lY / lD; // vtxY
      deriv[5][0] =   lZ / lD; // vtxZ


      TMatrixF tmp(6, 1);
      tmp.Mult(Cov, deriv);

      TMatrixF result(1, 1);
      result.Mult(deriv.T(), tmp);

      vertexDistanceErr = sqrt(result[0][0]);
      return lD;
    }

    double vertexDistance(const Particle* part)
    {
      double vertexDistanceError = std::numeric_limits<float>::quiet_NaN();
      if (!part->hasExtraInfo("prodVertX") || !part->hasExtraInfo("prodVertY") || !part->hasExtraInfo("prodVertZ")) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      return getVertexDistance(part, part, vertexDistanceError);
    }

    double vertexDistanceErr(const Particle* part)
    {
      double vertexDistanceError = std::numeric_limits<float>::quiet_NaN();
      if (!part->hasExtraInfo("prodVertX") || !part->hasExtraInfo("prodVertY") || !part->hasExtraInfo("prodVertZ")) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      getVertexDistance(part, part, vertexDistanceError);
      return vertexDistanceError;
    }

    double vertexDistanceSignificance(const Particle* part)
    {
      double vertexDistanceError = std::numeric_limits<float>::quiet_NaN();
      if (!part->hasExtraInfo("prodVertX") || !part->hasExtraInfo("prodVertY") || !part->hasExtraInfo("prodVertZ")) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      return getVertexDistance(part, part, vertexDistanceError) / vertexDistanceError;
    }

    Manager::FunctionPtr flightTimeOfDaughter(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      int grandDaughterNumber = -1;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of flightTimeOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        try {
          grandDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2WARNING("Second argument of flightTimeOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, grandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightTimeErr = 0.0;
          if (grandDaughterNumber > -1)
          {
            if (grandDaughterNumber < (int)daughter->getNDaughters()) {
              return getFlightInfoBtw(particle, daughter->getDaughter(grandDaughterNumber), flightTimeErr, "time", true);
            }
          } else {
            return getFlightInfoBtw(particle, daughter, flightTimeErr, "time");
          };
          return std::numeric_limits<float>::quiet_NaN();
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // Flight time uncertainty
    Manager::FunctionPtr flightTimeOfDaughterErr(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      int grandDaughterNumber = -1;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of flightTimeOfDaughterErr function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        try {
          grandDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2WARNING("Second argument of flightTimeOfDaughterErr function must be integer!");
          return nullptr;
        }
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, grandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightTimeErr = 0.0;
          if (grandDaughterNumber > -1)
          {
            if (grandDaughterNumber < (int)daughter->getNDaughters()) {
              getFlightInfoBtw(particle, daughter->getDaughter(grandDaughterNumber), flightTimeErr, "time", true);
              return flightTimeErr;
            }
          } else {
            getFlightInfoBtw(particle, daughter, flightTimeErr, "time");
            return flightTimeErr;
          };
          return std::numeric_limits<float>::quiet_NaN();
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    Manager::FunctionPtr flightDistanceOfDaughter(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      int grandDaughterNumber = -1;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of flightDistanceOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        try {
          grandDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2WARNING("Second argument of flightDistanceOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, grandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightDistanceErr = 0.0;
          if (grandDaughterNumber > -1)
          {
            if (grandDaughterNumber < (int)daughter->getNDaughters()) {
              return getFlightInfoBtw(particle, daughter->getDaughter(grandDaughterNumber), flightDistanceErr, "distance", true);
            }
          } else {
            return getFlightInfoBtw(particle, daughter, flightDistanceErr, "distance");
          };
          return std::numeric_limits<float>::quiet_NaN();
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // Flight distance uncertainty
    Manager::FunctionPtr flightDistanceOfDaughterErr(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      int grandDaughterNumber = -1;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of flightDistanceOfDaughterErr function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        try {
          grandDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2WARNING("Second argument of flightDistanceOfDaughterErr function must be integer!");
          return nullptr;
        }
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, grandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightDistanceErr = 0.0;
          if (grandDaughterNumber > -1)
          {
            if (grandDaughterNumber < (int)daughter->getNDaughters()) {
              getFlightInfoBtw(particle, daughter->getDaughter(grandDaughterNumber), flightDistanceErr, "distance", true);
              return flightDistanceErr;
            }
          } else {
            getFlightInfoBtw(particle, daughter, flightDistanceErr, "distance");
            return flightDistanceErr;
          };
          return std::numeric_limits<float>::quiet_NaN();
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // Distance between mother and daughter vertices
    Manager::FunctionPtr vertexDistanceOfDaughter(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      bool prodVertIsIP = true;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of vertexDistanceOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        prodVertIsIP = false;
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, prodVertIsIP](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter = particle->getDaughter(daughterNumber);
          double vertexDistanceErr = std::numeric_limits<float>::quiet_NaN();
          return getVertexDistance(particle, daughter, vertexDistanceErr, prodVertIsIP);
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // Uncertainty on distance between mother and daughter vertices
    Manager::FunctionPtr vertexDistanceOfDaughterErr(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      bool prodVertIsIP = true;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of vertexDistanceOfDaughterErr function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        prodVertIsIP = false;
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, prodVertIsIP](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter = particle->getDaughter(daughterNumber);
          double vertexDistanceErr = std::numeric_limits<float>::quiet_NaN();
          getVertexDistance(particle, daughter, vertexDistanceErr, prodVertIsIP);
          return vertexDistanceErr;
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // Significance of distance between mother and daughter vertices
    Manager::FunctionPtr vertexDistanceOfDaughterSignificance(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      bool prodVertIsIP = true;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of vertexDistanceOfDaughterSignificance function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        prodVertIsIP = false;
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, prodVertIsIP](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle* daughter = particle->getDaughter(daughterNumber);
          double vertexDistanceErr = 0.0;
          return getVertexDistance(particle, daughter, vertexDistanceErr, prodVertIsIP) / vertexDistanceErr;
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // MC variables

    double mcFlightDistance(const Particle* particle)
    {
      if (particle == nullptr)
        return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL

      const MCParticle* mcparticle = particle->getMCParticle();

      if (mcparticle == nullptr)
        return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL

      return getMCFlightInfoBtw(mcparticle, "distance");
    }

    double mcFlightTime(const Particle* particle)
    {
      if (particle == nullptr)
        return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL

      const MCParticle* mcparticle = particle->getMCParticle();

      if (mcparticle == nullptr)
        return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL


      return getMCFlightInfoBtw(mcparticle, "time");
    }

    Manager::FunctionPtr mcFlightDistanceOfDaughter(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      int grandDaughterNumber = -1;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of mcFlightDistanceOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        try {
          grandDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2WARNING("Second argument of mcFlightDistanceOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, grandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) || !particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle*  daughterReco = particle->getDaughter(daughterNumber);
          //get the MC DAUGHTER
          const MCParticle*  daughter = daughterReco->getMCParticle();

          double flightDistanceMC =  std::numeric_limits<float>::quiet_NaN();
          if (grandDaughterNumber > -1 && grandDaughterNumber < (int)daughterReco->getNDaughters())
          {
            // Compute value between mother and granddaughter
            const MCParticle*  gdaughter = daughterReco->getDaughter(grandDaughterNumber)->getMCParticle();

            if (gdaughter != nullptr)
              flightDistanceMC = getMCFlightInfoBtw(gdaughter, "distance");
          } else {
            // Compute value between mother and daughter
            if (daughter != nullptr)
              flightDistanceMC = getMCFlightInfoBtw(daughter, "distance");
          };
          return flightDistanceMC;
        }; // Lambda function END
        return func;
      }
      B2WARNING("Incorrect parameters of mcFlightDistanceOfDaughter!");
      return nullptr;
    }

    Manager::FunctionPtr mcFlightTimeOfDaughter(const std::vector<std::string>& arguments)
    {
      int daughterNumber = -1;
      int grandDaughterNumber = -1;
      if (arguments.size() == 1 || arguments.size() == 2) {
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2WARNING("First argument of flightTimeOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (arguments.size() == 2) {
        try {
          grandDaughterNumber = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2WARNING("Second argument of mcFlightTimeOfDaughter function must be integer!");
          return nullptr;
        }
      }
      if (daughterNumber > -1) {
        auto func = [daughterNumber, grandDaughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return std::numeric_limits<float>::quiet_NaN(); // Initial particle is nullptr
          if (daughterNumber >= int(particle->getNDaughters()) || !particle->getDaughter(daughterNumber))
            return std::numeric_limits<float>::quiet_NaN(); // Daughter number or daughters are inconsistent
          const Particle*  daughterReco = particle->getDaughter(daughterNumber);
          //get the MC DAUGHTER
          const MCParticle*  daughter = daughterReco->getMCParticle();
          // daughter MOMENTUM

          double flightTimeMC = std::numeric_limits<float>::quiet_NaN();
          if (grandDaughterNumber > -1 && grandDaughterNumber < (int)daughterReco->getNDaughters())
          {
            const MCParticle*  gdaughter = daughterReco->getDaughter(grandDaughterNumber)->getMCParticle();
            if (gdaughter != nullptr)
              flightTimeMC = getMCFlightInfoBtw(gdaughter, "time");
          } else  {
            if (daughter != nullptr)
              flightTimeMC = getMCFlightInfoBtw(daughter, "time");
          };
          return flightTimeMC;
        }; // Lambda function END
        return func;
      }
      B2WARNING("Incorrect parameters of mcFlightTimeOfDaughter!");
      return nullptr;
    }


    VARIABLE_GROUP("Flight Information");
    REGISTER_VARIABLE("flightTime", flightTime,
                      "Returns the flight time of particle. If a treeFit has been performed the flight time calculated by TreeFitter is returned. Otherwise if a beam constrained rave fit has been performed the production vertex set by rave and the decay vertex are used to calculate the flight time. If neither fit has been performed the i.p. is taken to be the production vertex.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("flightDistance", flightDistance,
                      "Returns the flight distance of particle. If a treeFit has been performed the flight distance calculated by TreeFitter is returned. Otherwise if a beam constrained rave fit has been performed the production vertex set by rave and the decay vertex are used to calculate the flight distance. If neither fit has been performed the i.p. is taken to be the production vertex.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("flightTimeErr", flightTimeErr,
                      "Returns the flight time error of particle. If a treeFit has been performed the flight time error calculated by TreeFitter is returned. Otherwise if a beam constrained rave fit has been performed the production vertex set by rave and the decay vertex are used to calculate the flight time error. If neither fit has been performed the i.p. is taken to be the production vertex.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("flightDistanceErr", flightDistanceErr,
                      "Returns the flight distance error of particle. If a treeFit has been performed the flight distance error calculated by TreeFitter is returned. Otherwise if a beam constrained rave fit has been performed the production vertex set by rave and the decay vertex are used to calculate the flight distance error. If neither fit has been performed the i.p. is taken to be the production vertex.",
                      Manager::VariableDataType::c_double);
    // Daughters
    REGISTER_VARIABLE("flightTimeOfDaughter(daughterN, gdaughterN = -1)", flightTimeOfDaughter,
                      "Returns the flight time between mother and daughter particle with daughterN index. If a treeFit has been performed the value calculated by treeFitter is returned. Otherwise the value is calculated using the decay vertices of the mother and daughter particle. If a second index granddaughterM is given the value is calculated between the mother and the Mth grandaughter (Mth daughter of Nth daughter).",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("flightTimeOfDaughterErr(daughterN, gdaughterN = -1)", flightTimeOfDaughterErr,
                      "Returns the flight time error between mother and daughter particle with daughterN index. If a treeFit has been performed the value calculated by treeFitter is returned. Otherwise the value is calculated using the decay vertices of the mother and daughter particle. If a second index granddaughterM is given the value is calculated between the mother and the Mth grandaughter (Mth daughter of Nth daughter).",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("flightDistanceOfDaughter(daughterN, gdaughterN = -1)", flightDistanceOfDaughter,
                      "Returns the flight distance between mother and daughter particle with daughterN index. If a treeFit has been performed the value calculated by treeFitter is returned. Otherwise the value is calculated using the decay vertices of the mother and daughter particle. If a second index granddaughterM is given the value is calculated between the mother and the Mth grandaughter (Mth daughter of Nth daughter).",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("flightDistanceOfDaughterErr(daughterN, gdaughterN = -1)", flightDistanceOfDaughterErr,
                      "Returns the flight distance error between mother and daughter particle with daughterN index. If a treeFit has been performed the value calculated by treeFitter is returned. Otherwise the value is calculated using the decay vertices of the mother and daughter particle. If a second index granddaughterM is given the value is calculated between the mother and the Mth grandaughter (Mth daughter of Nth daughter).",
                      Manager::VariableDataType::c_double);
    // MC Info
    REGISTER_VARIABLE("mcFlightDistance", mcFlightDistance,
                      "Returns the MC flight distance of the particle", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("mcFlightTime", mcFlightTime,
                      "Returns the MC flight time of the particle", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("mcFlightDistanceOfDaughter(daughterN, gdaughterN = -1)", mcFlightDistanceOfDaughter,
                      "Returns the MC flight distance between mother and daughter particle using generated info", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("mcFlightTimeOfDaughter(daughterN, gdaughterN = -1)", mcFlightTimeOfDaughter,
                      "Returns the MC flight time between mother and daughter particle using generated info", Manager::VariableDataType::c_double);
    //Vertex Distance
    REGISTER_VARIABLE("vertexDistance", vertexDistance,
                      "Returns the distance between the production and decay vertex of a particle. Returns NaN if particle has no production or decay vertex.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("vertexDistanceErr", vertexDistanceErr,
                      "Returns the uncertainty on the distance between the production and decay vertex of a particle. Returns NaN if particle has no production or decay vertex.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("vertexDistanceSignificance", vertexDistanceSignificance,
                      "Returns the distance between the production and decay vertex of a particle in units of the uncertainty on this value, i.e. the significance of the vertex separation.",
                      Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("vertexDistanceOfDaughter(daughterN, option = '')", vertexDistanceOfDaughter,
                      "If any second argument is provided it returns the distance between the decay vertices of the particle and of its daughter with index daughterN.\n"
                      "Otherwise, it is assumed that the particle has a production vertex (typically the IP) which is used to calculate the distance to the daughter's decay vertex.\n"
                      "Returns NaN in case anything goes wrong.", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("vertexDistanceOfDaughterErr(daughterN, option = '')", vertexDistanceOfDaughterErr,
                      "If any second argument is provided it returns the uncertainty on the distance between the decay vertices of the particle and of its daughter with index daughterN.\n"
                      "Otherwise, it is assumed that the particle has a production vertex (typically the IP) with a corresponding covariance matrix to calculate the uncertainty on the distance to the daughter's decay vertex.\n"
                      "Returns NaN in case anything goes wrong.", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("vertexDistanceOfDaughterSignificance(daughterN, option = '')", vertexDistanceOfDaughterSignificance,
                      "If any second argument is provided it returns the distance between the decay vertices of the particle and of its daughter with index daughterN in units of the uncertainty on this value.\n"
                      "Otherwise, it is assumed that the particle has a production vertex (typically the IP) with a corresponding covariance matrix and the significance of the separation to this vertex is calculated.",
                      Manager::VariableDataType::c_double);
    // GrandDaughters
    //REGISTER_VARIABLE("flightDistanceOfGrandDaughter(daughterN)", flightDistanceOfGrandDaughter,
    //                  "Returns the flight distance between mother and daughter particle");
  }
} // Belle2 namespace
