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
    double particleMCX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().X();
      }
      return -999;
    }

    double particleMCY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Y();
      }
      return -999;
    }

    double particleMCZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Z();
      }
      return -999;
    }




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
    // vertex or POCA in respect to IP ------------------------------

    double particleDX(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).X();
    }

    double particleDY(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Y();
    }

    double particleDZ(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Z();
    }

    inline double getParticleUncertaintyByIndex(const Particle* part, unsigned int index)
    {
      if (!part) {
        B2FATAL("The particle provide does not exist.");
      }
      const auto& errMatrix = part->getVertexErrorMatrix();
      return std::sqrt(errMatrix(index, index));
    }

    double particleDXUncertainty(const Particle* part)
    {
      return getParticleUncertaintyByIndex(part, 0);
    }

    double particleDYUncertainty(const Particle* part)
    {
      return getParticleUncertaintyByIndex(part, 1);
    }

    double particleDZUncertainty(const Particle* part)
    {
      return getParticleUncertaintyByIndex(part, 2);
    }

    double particleDRho(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Perp();
    }

    double particleDPhi(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Phi();
    }

    double particleDCosTheta(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).CosTheta();
    }

    double particleDistance(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Mag();
    }

    double particleDistanceSignificance(const Particle* part)
    {
      // significance is defined as s = r/sigma_r, therefore:
      // s &= \frac{r}{\sqrt{ \sum_{ij} \frac{\partial r}{x_i} V_{ij} \frac{\partial r}{x_j}}}
      //   &= \frac{r^2}{\sqrt{\vec{x}V\vec{x}}}
      // where:
      // r &= \sqrt{\vec{x}*\vec{x}}
      // and V_{ij} is the covariance matrix
      const auto& frame = ReferenceFrame::GetCurrent();
      const auto& vertex = frame.getVertex(part);
      const auto& vertexErr = frame.getVertexErrorMatrix(part->getVertexErrorMatrix());
      auto denominator = vertex * (vertexErr * vertex);
      if (denominator <= 0)
        return -1;
      return vertex.Mag2() / sqrt(denominator);
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
    REGISTER_VARIABLE("mcX", particleMCX,
                      "Returns the x position of decay vertex of matched generated particle. Returns -999 if particle has no matched generated particle.");
    REGISTER_VARIABLE("mcY", particleMCY,
                      "Returns the y position of decay vertex of matched generated particle.");
    REGISTER_VARIABLE("mcZ", particleMCZ,
                      "Returns the z position of decay vertex of matched generated particle.");
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

    // Decay vertex position
    REGISTER_VARIABLE("distance", particleDistance,
                      "3D distance relative to interaction point");
    REGISTER_VARIABLE("significanceOfDistance", particleDistanceSignificance,
                      "significance of distance relative to interaction point(-1 in case of numerical problems)");
    REGISTER_VARIABLE("dx", particleDX, "x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "z in respect to IP");
    REGISTER_VARIABLE("x", particleDX,
                      "x coordinate of vertex in case of composite particle, or point of the closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("y", particleDY, "y coordinate of vertex");
    REGISTER_VARIABLE("z", particleDZ, "z coordinate of vertex");
    REGISTER_VARIABLE("x_uncertainty", particleDXUncertainty, "uncertainty on x");
    REGISTER_VARIABLE("y_uncertainty", particleDYUncertainty, "uncertainty on y");
    REGISTER_VARIABLE("z_uncertainty", particleDZUncertainty, "uncertainty on z");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP");
    REGISTER_VARIABLE("dphi", particleDPhi, "vertex azimuthal angle in degrees in respect to IP");
    REGISTER_VARIABLE("dcosTheta", particleDCosTheta, "vertex polar angle in respect to IP");
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
