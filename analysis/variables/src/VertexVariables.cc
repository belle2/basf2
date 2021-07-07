/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/variables/VertexVariables.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

#include <TMatrixFSym.h>
#include <TVector3.h>

#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>


namespace Belle2 {
  class Particle;

  namespace Variable {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();

    // Generated vertex information
    double mcDecayVertexX(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getDecayVertex().X();
    }

    double mcDecayVertexY(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getDecayVertex().Y();
    }

    double mcDecayVertexZ(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getDecayVertex().Z();
    }

    double mcDecayVertexRho(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getDecayVertex().Perp();
    }

    TVector3 getMcDecayVertexFromIP(const MCParticle* mcparticle)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition());
    }

    double mcDecayVertexFromIPX(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcDecayVertexFromIP(mcparticle).X();
    }

    double mcDecayVertexFromIPY(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcDecayVertexFromIP(mcparticle).Y();
    }

    double mcDecayVertexFromIPZ(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcDecayVertexFromIP(mcparticle).Z();
    }

    double mcDecayVertexFromIPRho(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcDecayVertexFromIP(mcparticle).Perp();
    }

    double mcDecayVertexFromIPDistance(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcDecayVertexFromIP(mcparticle).Mag();
    }

    double mcProductionVertexX(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getProductionVertex().X();
    }

    double mcProductionVertexY(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getProductionVertex().Y();
    }

    double mcProductionVertexZ(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return mcparticle->getProductionVertex().Z();
    }

    TVector3 getMcProductionVertexFromIP(const MCParticle* mcparticle)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      return frame.getVertex(mcparticle->getProductionVertex() - beamSpotDB->getIPPosition());
    }

    double mcProductionVertexFromIPX(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcProductionVertexFromIP(mcparticle).X();
    }

    double mcProductionVertexFromIPY(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcProductionVertexFromIP(mcparticle).Y();
    }

    double mcProductionVertexFromIPZ(const Particle* part)
    {
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle) return realNaN;
      return getMcProductionVertexFromIP(mcparticle).Z();
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

    TVector3 getVertexD(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = part->getTrackFitResult();
      if (!trackFit)
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition());

      UncertainHelix helix = trackFit->getUncertainHelix();
      helix.passiveMoveBy(beamSpotDB->getIPPosition());
      return frame.getVertex(helix.getPerigee());
    }


    double particleDX(const Particle* part)
    {
      return getVertexD(part).X();
    }

    double particleDY(const Particle* part)
    {
      return getVertexD(part).Y();
    }

    double particleDZ(const Particle* part)
    {
      return getVertexD(part).Z();
    }

    double particleDRho(const Particle* part)
    {
      return getVertexD(part).Perp();
    }

    double particleDPhi(const Particle* part)
    {
      return getVertexD(part).Phi();
    }

    double particleDCosTheta(const Particle* part)
    {
      return getVertexD(part).CosTheta();
    }

    double particleDistance(const Particle* part)
    {
      return getVertexD(part).Mag();
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
      const TVector3& vertex = frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition());
      const TMatrixFSym& vertexErr = frame.getVertexErrorMatrix(static_cast<TMatrixDSym>(part->getVertexErrorMatrix()) +
                                                                beamSpotDB->getCovVertex());
      const double denominator = vertex * (vertexErr * vertex);
      if (denominator <= 0) return realNaN;

      return vertex.Mag2() / std::sqrt(denominator);
    }

    // Production vertex position

    double particleProductionX(const Particle* part)
    {
      if (!part->hasExtraInfo("prodVertX")) return realNaN;
      return part->getExtraInfo("prodVertX");
    }

    double particleProductionY(const Particle* part)
    {
      if (!part->hasExtraInfo("prodVertY")) return realNaN;
      return part->getExtraInfo("prodVertY");
    }

    double particleProductionZ(const Particle* part)
    {
      if (!part->hasExtraInfo("prodVertZ")) return realNaN;
      return part->getExtraInfo("prodVertZ");
    }

    // Production vertex covariance matrix
    Manager::FunctionPtr particleProductionCovElement(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2FATAL("Number of arguments of prodVertexCov function is incorrect!");
      }

      int ielement = -1;
      int jelement = -1;
      try {
        ielement = Belle2::convertString<int>(arguments[0]);
        jelement = Belle2::convertString<int>(arguments[1]);
      } catch (std::invalid_argument&) {
        B2ERROR("Arguments of prodVertexCov function must be integer!");
        return nullptr;
      }

      if (std::min(ielement, jelement) < 0 || std::max(ielement, jelement) > 2) {
        B2ERROR("Range of indexes of prodVertexCov function is incorrect!");
        return nullptr;
      }

      const std::vector<char> names = {'x', 'y', 'z'};
      const std::string prodVertS = Form("prodVertS%c%c", names[ielement] , names[jelement]);

      return [prodVertS](const Particle * part) -> double {
        if (!part->hasExtraInfo(prodVertS)) return realNaN;
        return part->getExtraInfo(prodVertS);
      };
    }

    double particleProductionXErr(const Particle* part)
    {
      if (!part->hasExtraInfo("prodVertSxx")) return realNaN;
      return std::sqrt(part->getExtraInfo("prodVertSxx"));
    }

    double particleProductionYErr(const Particle* part)
    {
      if (!part->hasExtraInfo("prodVertSyy")) return realNaN;
      return std::sqrt(part->getExtraInfo("prodVertSyy"));
    }

    double particleProductionZErr(const Particle* part)
    {
      if (!part->hasExtraInfo("prodVertSzz")) return realNaN;
      return std::sqrt(part->getExtraInfo("prodVertSzz"));
    }

    VARIABLE_GROUP("Vertex Information");
    // Generated vertex information
    REGISTER_VARIABLE("mcDecayVertexX", mcDecayVertexX,
                      "Returns the x position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexY", mcDecayVertexY,
                      "Returns the y position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexZ", mcDecayVertexZ,
                      "Returns the z position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexRho", mcDecayVertexRho,
                      "Returns the transverse position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexFromIPX", mcDecayVertexFromIPX,
                      "Returns the x position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexFromIPY", mcDecayVertexFromIPY,
                      "Returns the y position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexFromIPZ", mcDecayVertexFromIPZ,
                      "Returns the z position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexFromIPRho", mcDecayVertexFromIPRho,
                      "Returns the transverse position of the decay vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexFromIPDistance", mcDecayVertexFromIPDistance,
                      "Returns the distance of the decay vertex of the matched generated particle from the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProductionVertexX", mcProductionVertexX,
                      "Returns the x position of production vertex of matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProductionVertexY", mcProductionVertexY,
                      "Returns the y position of production vertex of matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProductionVertexZ", mcProductionVertexZ,
                      "Returns the z position of production vertex of matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProductionVertexFromIPX", mcProductionVertexFromIPX,
                      "Returns the x position of the production vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProductionVertexFromIPY", mcProductionVertexFromIPY,
                      "Returns the y position of the production vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcProductionVertexFromIPZ", mcProductionVertexFromIPZ,
                      "Returns the z position of the production vertex of the matched generated particle wrt the IP. Returns nan if the particle has no matched generated particle.");

    // Decay vertex position
    REGISTER_VARIABLE("distance", particleDistance,
                      R"DOC(3D distance between the IP and the particle decay vertex, if available.

In case the particle has been created from a track, the distance is defined between the POCA and IP.
If the particle is built from an ECL cluster, the decay vertex is set to the nominal IP. 
If the particle is created from a KLM cluster, the distance is calculated between the IP and the cluster itself.)DOC");

    REGISTER_VARIABLE("significanceOfDistance", particleDistanceSignificance,
                      "significance of distance from vertex or POCA to interaction point(-1 in case of numerical problems)");
    REGISTER_VARIABLE("dx", particleDX, "vertex or POCA in case of tracks x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "vertex or POCA in case of tracks y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "vertex or POCA in case of tracks z in respect to IP");
    REGISTER_VARIABLE("x", particleX,
                      "x coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("y", particleY,
                      "y coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("z", particleZ,
                      "z coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("x_uncertainty", particleDXUncertainty, "uncertainty on x (measured with respect to the origin)");
    REGISTER_VARIABLE("y_uncertainty", particleDYUncertainty, "uncertainty on y (measured with respect to the origin)");
    REGISTER_VARIABLE("z_uncertainty", particleDZUncertainty, "uncertainty on z (measured with respect to the origin)");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP for a vertex; track d0 relative to IP for a track.");
    REGISTER_VARIABLE("dphi", particleDPhi, "vertex azimuthal angle of the vertex or POCA in degrees in respect to IP");
    REGISTER_VARIABLE("dcosTheta", particleDCosTheta, "vertex or POCA polar angle in respect to IP");
    // Production vertex position
    REGISTER_VARIABLE("prodVertexX", particleProductionX,
                      "Returns the x position of particle production vertex. Returns NaN if particle has no production vertex.");
    REGISTER_VARIABLE("prodVertexY", particleProductionY,
                      "Returns the y position of particle production vertex.");
    REGISTER_VARIABLE("prodVertexZ", particleProductionZ,
                      "Returns the z position of particle production vertex.");
    // Production vertex covariance matrix
    REGISTER_VARIABLE("prodVertexCov(i,j)", particleProductionCovElement,
                      "Returns the ij covariance matrix component of particle production vertex, arguments i,j should be 0, 1 or 2. Returns NaN if particle has no production covariance matrix.");
    REGISTER_VARIABLE("prodVertexXErr", particleProductionXErr,
                      "Returns the x position uncertainty of particle production vertex. Returns NaN if particle has no production vertex.");
    REGISTER_VARIABLE("prodVertexYErr", particleProductionYErr,
                      "Returns the y position uncertainty of particle production vertex.");
    REGISTER_VARIABLE("prodVertexZErr", particleProductionZErr,
                      "Returns the z position uncertainty of particle production vertex.");

  }
}
