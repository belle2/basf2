/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi, Sviatoslav Bilokin                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/VariableManager/VertexVariables.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/utility/ReferenceFrame.h>
#include <TMatrixFSym.h>
#include <TVector3.h>

namespace Belle2 {
  class Particle;

  namespace Variable {

    // Generated vertex information

    double particleMCDistance(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex()).Mag();
      }
      return -999;
    }

    double particleMCRho(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex()).Perp();
      }
      return -999;
    }

    double particleMCProductionX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getVertex().X();
      }
      return -999;
    }

    double particleMCProductionY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getVertex().Y();
      }
      return -999;
    }

    double particleMCProductionZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getVertex().Z();
      }
      return -999;
    }

    // Production vertex position

    double particleProductionX(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertX")) {
        return part->getExtraInfo("prodVertX");
      }
      return -999;
    }

    double particleProductionY(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertY")) {
        return part->getExtraInfo("prodVertY");
      }
      return -999;
    }

    double particleProductionZ(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertZ")) {
        return part->getExtraInfo("prodVertZ");
      }
      return -999;
    }

    // Production vertex covariance matrix

    double particleProductionCovXX(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSxx")) {
        return part->getExtraInfo("prodVertSxx");
      }
      return -999;
    }

    double particleProductionCovXY(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSxy")) {
        return part->getExtraInfo("prodVertSxy");
      }
      return -999;
    }

    double particleProductionCovXZ(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSxz")) {
        return part->getExtraInfo("prodVertSxz");
      }
      return -999;
    }

    double particleProductionCovYX(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSyx")) {
        return part->getExtraInfo("prodVertSyx");
      }
      return -999;
    }

    double particleProductionCovYY(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSyy")) {
        return part->getExtraInfo("prodVertSyy");
      }
      return -999;
    }

    double particleProductionCovYZ(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSyz")) {
        return part->getExtraInfo("prodVertSyz");
      }
      return -999;
    }

    double particleProductionCovZX(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSzx")) {
        return part->getExtraInfo("prodVertSzx");
      }
      return -999;
    }

    double particleProductionCovZY(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSzy")) {
        return part->getExtraInfo("prodVertSzy");
      }
      return -999;
    }

    double particleProductionCovZZ(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSzz")) {
        return part->getExtraInfo("prodVertSzz");
      }
      return -999;
    }

    VARIABLE_GROUP("Vertex Information");
    // Generated vertex information
    REGISTER_VARIABLE("mcDistance", particleMCDistance,
                      "Returns the distance to IP of decay vertex of matched generated particle. Returns -999 if particle has no matched generated particle.");
    REGISTER_VARIABLE("mcRho", particleMCRho,
                      "Returns the transverse position of decay vertex of matched generated particle. Returns -999 if particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexX", particleMCProductionX,
                      "Returns the X position of production vertex of matched generated particle. Returns -999 if particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexY", particleMCProductionY,
                      "Returns the Y position of production vertex of matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexZ", particleMCProductionZ,
                      "Returns the Z position of production vertex of matched generated particle.");

    // Production vertex position
    REGISTER_VARIABLE("prodVertexX", particleProductionX,
                      "Returns the X position of particle production vertex. Returns -999 if particle has no production vertex.");
    REGISTER_VARIABLE("prodVertexY", particleProductionY,
                      "Returns the Y position of particle production vertex.");
    REGISTER_VARIABLE("prodVertexZ", particleProductionZ,
                      "Returns the Z position of particle production vertex.");
    // Production vertex covariance matrix
    REGISTER_VARIABLE("prodVertexCovXX", particleProductionCovXX,
                      "Returns the XX component of particle production covariance matrix. Returns -999 if particle has no production covariance matrix.");
    REGISTER_VARIABLE("prodVertexCovXY", particleProductionCovXY,
                      "Returns the XY component of particle production covariance matrix.");
    REGISTER_VARIABLE("prodVertexCovXZ", particleProductionCovXZ,
                      "Returns the XZ component of particle production covariance matrix.");

    REGISTER_VARIABLE("prodVertexCovYX", particleProductionCovYX,
                      "Returns the YX component of particle production covariance matrix.");
    REGISTER_VARIABLE("prodVertexCovYY", particleProductionCovYY,
                      "Returns the YY component of particle production covariance matrix.");
    REGISTER_VARIABLE("prodVertexCovYZ", particleProductionCovYZ,
                      "Returns the YZ component of particle production covariance matrix.");

    REGISTER_VARIABLE("prodVertexCovZX", particleProductionCovZX,
                      "Returns the ZX component of particle production covariance matrix.");
    REGISTER_VARIABLE("prodVertexCovZY", particleProductionCovZY,
                      "Returns the ZY component of particle production covariance matrix.");
    REGISTER_VARIABLE("prodVertexCovZZ", particleProductionCovZZ,
                      "Returns the ZZ component of particle production covariance matrix.");

  }
}
