
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc, Thomas Keck                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/




#include <analysis/VariableManager/FlightInfoVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/utility/ReferenceFrame.h>
#include <framework/utilities/Conversion.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <mdst/dataobjects/MCParticle.h>
#include <vector>
#include <TLorentzVector.h>
#include <TMatrixFSym.h>
#include <TVector3.h>

namespace Belle2 {
  class Particle;

  namespace Variable {

    // Helper function for flight distance and its uncertainty (provided as it is)
    inline double getFlightInfoDistanceBtw(const Particle* particle, const Particle* daughter, double& distanceErr)
    {
      double mumvtxX = particle->getX();
      double mumvtxY = particle->getY();
      double mumvtxZ = particle->getZ();
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
      //covariance matrix of momentum and vertex for the Dz
      //ORDER = px,py,pz,E,x,y,z
      TMatrixFSym dauCov = daughter->getMomentumVertexErrorMatrix();
      TMatrixFSym mumCov = particle->getVertexErrorMatrix();   //order: x,y,z

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

      distanceErr = sqrt(result[0][0]);
      return fD;
    }
    // Helper function for flight time and its uncertainty (provided as it is)
    inline double getFlightInfoTimeBtw(const Particle* particle, const Particle* daughter, double& timeErr)
    {
      double mumvtxX = particle->getX();
      double mumvtxY = particle->getY();
      double mumvtxZ = particle->getZ();
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
      timeErr = sqrt(result[0][0]);
      return fT;
    }
    Manager::FunctionPtr flightTimeOfDaughter(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of flightTimeOfDaughter function must be integer!");
          return nullptr;
        }
        auto func = [daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999; // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return -999; // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightTimeErr = 0.0;
          double flightTime = getFlightInfoTimeBtw(particle, daughter, flightTimeErr);
          return flightTime;
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    // Flight time uncertainty
    Manager::FunctionPtr flightTimeOfDaughterErr(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of flightTimeOfDaughterErr function must be integer!");
          return nullptr;
        }
        auto func = [daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999; // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return -999; // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightTimeErr = 0.0;
          getFlightInfoTimeBtw(particle, daughter, flightTimeErr);
          return flightTimeErr;
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }
    Manager::FunctionPtr flightDistanceOfDaughter(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of flightDistanceOfDaughter function must be integer!");
          return nullptr;
        }
        auto func = [daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999; // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return -999; // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          double flightDistanceErr = 0.0;
          double flightDistance = getFlightInfoDistanceBtw(particle, daughter, flightDistanceErr);
          return flightDistance;
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }

    //Flight distance uncertainty
    Manager::FunctionPtr flightDistanceOfDaughterErr(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        int daughterNumber = 0;
        try {
          daughterNumber = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("First argument of flightDistanceOfDaughterErr function must be integer!");
          return nullptr;
        }
        auto func = [daughterNumber](const Particle * particle) -> double {
          if (particle == nullptr)
            return -999; // Initial particle is NULL
          if (daughterNumber >= int(particle->getNDaughters()) && particle->getDaughter(daughterNumber))
            return -999; // Daughter number or daughters are inconsistent
          const Particle* daughter =  particle->getDaughter(daughterNumber);
          //const auto& frame = ReferenceFrame::GetCurrent();
          //return frame.getVertex(particle->getDaughter(daughterNumber)).Mag();
          double flightDistanceErr = 0.0;
          getFlightInfoDistanceBtw(particle, daughter, flightDistanceErr);
          return flightDistanceErr;
        }; // Lambda function END
        return func;
      }
      return nullptr;
    }
    // Daughters
    VARIABLE_GROUP("FlightInformation");
    REGISTER_VARIABLE("flightTimeOfDaughter(daughterN)", flightTimeOfDaughter,
                      "Returns the flight time between mother and daughter particle");
    REGISTER_VARIABLE("flightTimeOfDaughterErr(daughterN)", flightTimeOfDaughterErr,
                      "Returns the flight time uncertainty between mother and daughter particle");
    REGISTER_VARIABLE("flightDistanceOfDaughter(daughterN)", flightDistanceOfDaughter,
                      "Returns the flight distance between mother and daughter particle");
    REGISTER_VARIABLE("flightDistanceOfDaughterErr(daughterN)", flightDistanceOfDaughterErr,
                      "Returns the flight distance uncertainty between mother and daughter particle");
    // GrandDaughters
    //REGISTER_VARIABLE("flightDistanceOfGrandDaughter(daughterN)", flightDistanceOfGrandDaughter,
    //                  "Returns the flight distance between mother and daughter particle");

  }
} // Belle2 namespace
