/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Luigi Li Gioi, Sviatoslav Bilokin                        *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/variables/VertexVariables.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/utility/ReferenceFrame.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <TMatrixFSym.h>
#include <TVector3.h>
#include <boost/format.hpp>

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
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCRho(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Perp();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCDX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCDY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCDZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCDRho(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Perp();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCDistance(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Mag();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCProductionX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getProductionVertex().X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCProductionY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getProductionVertex().Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCProductionZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getProductionVertex().Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCProductionDX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getProductionVertex() - beamSpotDB->getIPPosition()).X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCProductionDY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getProductionVertex() - beamSpotDB->getIPPosition()).Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleMCProductionDZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getProductionVertex() - beamSpotDB->getIPPosition()).Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    // vertex or POCA in respect to origin ------------------------------

    double particleX(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).X();
    }

    double particleY(const Particle* part)
    {
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part).Y();
    }

    double particleZ(const Particle* part)
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
      // uncertainty on x (with respect to the origin)
      return getParticleUncertaintyByIndex(part, 0);
    }

    double particleDYUncertainty(const Particle* part)
    {
      // uncertainty on y (with respect to the origin)
      return getParticleUncertaintyByIndex(part, 1);
    }

    double particleDZUncertainty(const Particle* part)
    {
      // uncertainty on z (with respect to the origin)
      return getParticleUncertaintyByIndex(part, 2);
    }

    //----------------------------------------------------------------------------------
    // vertex or POCA in respect to measured IP
    double particleDX(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).X();
    }

    double particleDY(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Y();
    }

    double particleDZ(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Z();
    }

    double particleDRho(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Perp();
    }

    double particleDPhi(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Phi();
    }

    double particleDCosTheta(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).CosTheta();
    }

    double particleDistance(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Mag();
    }

    double particleDistanceSignificance(const Particle* part)
    {
      // significance is defined as s = r/sigma_r, therefore:
      // s &= \frac{r}{\sqrt{ \sum_{ij} \frac{\partial r}{x_i} V_{ij} \frac{\partial r}{x_j}}}
      //   &= \frac{r^2}{\sqrt{\vec{x}V\vec{x}}}
      // where:
      // r &= \sqrt{\vec{x}*\vec{x}}
      // and V_{ij} is the covariance matrix
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      const auto& vertex = frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition());
      const auto& vertexErr = frame.getVertexErrorMatrix(static_cast<TMatrixDSym>(part->getVertexErrorMatrix()) +
                                                         beamSpotDB->getCovVertex());
      auto denominator = vertex * (vertexErr * vertex);
      if (denominator <= 0) {
        return -999;
      }
      return vertex.Mag2() / std::sqrt(denominator);
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
    Manager::FunctionPtr particleProductionCovElement(const std::vector<std::string>& arguments)
    {
      int ielement = -1;
      int jelement = -1;
      if (arguments.size() == 2) {
        try {
          ielement = Belle2::convertString<int>(arguments[0]);
          jelement = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2WARNING("Arguments of prodVertexCov function must be integer!");
          return nullptr;
        }
      }
      if (ielement > -1 && jelement > -1 && ielement < 3 && jelement < 3) {
        auto func = [ielement, jelement](const Particle * part) -> double {
          std::vector<std::string> names = {"x", "y", "z"};
          std::string prodVertS =  boost::str(boost::format("prodVertS%s%s") % names[ielement] % names[jelement]);
          if (part->hasExtraInfo(prodVertS))
          {
            return part->getExtraInfo(prodVertS);
          }
          return -999;
        };
        return func;
      }
      B2WARNING("Arguments of prodVertexCov function are incorrect!");
      return nullptr;
    }

    double particleProductionXErr(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSxx")) {
        return std::sqrt(part->getExtraInfo("prodVertSxx"));
      }
      return -999;
    }

    double particleProductionYErr(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSyy")) {
        return std::sqrt(part->getExtraInfo("prodVertSyy"));
      }
      return -999;
    }

    double particleProductionZErr(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSzz")) {
        return std::sqrt(part->getExtraInfo("prodVertSzz"));
      }
      return -999;
    }

    VARIABLE_GROUP("Vertex Information");
    // Generated vertex information
    REGISTER_VARIABLE("mcX", particleMCX,
                      "Returns the x position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcY", particleMCY,
                      "Returns the y position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcZ", particleMCZ,
                      "Returns the z position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcRho", particleMCRho,
                      "Returns the transverse position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDX", particleMCDX,
                      "Returns the x position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDY", particleMCDY,
                      "Returns the y position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDZ", particleMCDZ,
                      "Returns the z position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDRho", particleMCDRho,
                      "Returns the transverse position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDistance", particleMCDistance,
                      "Returns the distance of the decay vertex of the matched generated particle from the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexX", particleMCProductionX,
                      "Returns the x position of production vertex of matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexY", particleMCProductionY,
                      "Returns the y position of production vertex of matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexZ", particleMCProductionZ,
                      "Returns the z position of production vertex of matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexDX", particleMCProductionDX,
                      "Returns the x position of the production vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexDY", particleMCProductionDY,
                      "Returns the y position of the production vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProdVertexDZ", particleMCProductionDZ,
                      "Returns the z position of the production vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");

    // Decay vertex position
    REGISTER_VARIABLE("distance", particleDistance, "3D distance relative to interaction point");
    REGISTER_VARIABLE("significanceOfDistance", particleDistanceSignificance,
                      "significance of distance relative to interaction point(-1 in case of numerical problems)");
    REGISTER_VARIABLE("dx", particleDX, "x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "z in respect to IP");
    REGISTER_VARIABLE("x", particleX,
                      "x coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("y", particleY,
                      "y coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("z", particleZ,
                      "z coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("x_uncertainty", particleDXUncertainty, "uncertainty on x (measured with respect to the origin)");
    REGISTER_VARIABLE("y_uncertainty", particleDYUncertainty, "uncertainty on y (measured with respect to the origin)");
    REGISTER_VARIABLE("z_uncertainty", particleDZUncertainty, "uncertainty on z (measured with respect to the origin)");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP");
    REGISTER_VARIABLE("dphi", particleDPhi, "vertex azimuthal angle in degrees in respect to IP");
    REGISTER_VARIABLE("dcosTheta", particleDCosTheta, "vertex polar angle in respect to IP");
    // Production vertex position
    REGISTER_VARIABLE("prodVertexX", particleProductionX,
                      "Returns the x position of particle production vertex. Returns -999 if particle has no production vertex.");
    REGISTER_VARIABLE("prodVertexY", particleProductionY,
                      "Returns the y position of particle production vertex.");
    REGISTER_VARIABLE("prodVertexZ", particleProductionZ,
                      "Returns the z position of particle production vertex.");
    // Production vertex covariance matrix
    REGISTER_VARIABLE("prodVertexCov(i,j)", particleProductionCovElement,
                      "Returns the ij covariance matrix component of particle production vertex, arguments i,j should be 0,1 or 2. Returns -999 if particle has no production covariance matrix.");
    REGISTER_VARIABLE("prodVertexXErr", particleProductionXErr,
                      "Returns the x position uncertainty of particle production vertex. Returns -999 if particle has no production vertex.");
    REGISTER_VARIABLE("prodVertexYErr", particleProductionYErr,
                      "Returns the y position uncertainty of particle production vertex.");
    REGISTER_VARIABLE("prodVertexZErr", particleProductionZErr,
                      "Returns the z position uncertainty of particle production vertex.");

  }
}
