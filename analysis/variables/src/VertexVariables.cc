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
#include <analysis/variables/TrackVariables.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <TMatrixFSym.h>
#include <TVector3.h>

#include <mdst/dbobjects/BeamSpot.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

namespace Belle2 {
  class Particle;

  namespace Variable {

    // Generated vertex information
    double mcDecayVertexX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexRho(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getDecayVertex().Perp();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexFromIPX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexFromIPY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexFromIPZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexFromIPRho(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Perp();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcDecayVertexFromIPDistance(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getDecayVertex() - beamSpotDB->getIPPosition()).Mag();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcProductionVertexX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getProductionVertex().X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcProductionVertexY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getProductionVertex().Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcProductionVertexZ(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        return mcparticle->getProductionVertex().Z();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcProductionVertexFromIPX(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getProductionVertex() - beamSpotDB->getIPPosition()).X();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcProductionVertexFromIPY(const Particle* part)
    {
      auto* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle) {
        static DBObjPtr<BeamSpot> beamSpotDB;
        const auto& frame = ReferenceFrame::GetCurrent();
        return frame.getVertex(mcparticle->getProductionVertex() - beamSpotDB->getIPPosition()).Y();
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double mcProductionVertexFromIPZ(const Particle* part)
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
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).X();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).X();
      }
    }

    double particleDY(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Y();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).Y();
      }
    }

    double particleDZ(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Z();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).Z();
      }
    }

    double particleDRho(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Perp();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).Perp();
      }
    }

    double particleDPhi(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Phi();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).Phi();
      }
    }

    double particleDCosTheta(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).CosTheta();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).CosTheta();
      }
    }

    double particleDistance(const Particle* part)
    {
      static DBObjPtr<BeamSpot> beamSpotDB;
      const auto& frame = ReferenceFrame::GetCurrent();
      auto trackFit = getTrackFitResultFromParticle(part);
      if (!trackFit) {
        return frame.getVertex(part->getVertex() - beamSpotDB->getIPPosition()).Mag();
      } else {
        UncertainHelix helix = trackFit->getUncertainHelix();
        helix.passiveMoveBy(beamSpotDB->getIPPosition());
        return frame.getVertex(helix.getPerigee()).Mag();
      }
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
        return std::numeric_limits<double>::quiet_NaN();
      }
      return vertex.Mag2() / std::sqrt(denominator);
    }

    // Production vertex position

    double particleProductionX(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertX")) {
        return part->getExtraInfo("prodVertX");
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleProductionY(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertY")) {
        return part->getExtraInfo("prodVertY");
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleProductionZ(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertZ")) {
        return part->getExtraInfo("prodVertZ");
      }
      return std::numeric_limits<double>::quiet_NaN();
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
        } catch (std::invalid_argument&) {
          B2ERROR("Arguments of prodVertexCov function must be integer!");
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
          return std::numeric_limits<double>::quiet_NaN();
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
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleProductionYErr(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSyy")) {
        return std::sqrt(part->getExtraInfo("prodVertSyy"));
      }
      return std::numeric_limits<double>::quiet_NaN();
    }

    double particleProductionZErr(const Particle* part)
    {
      if (part->hasExtraInfo("prodVertSzz")) {
        return std::sqrt(part->getExtraInfo("prodVertSzz"));
      }
      return std::numeric_limits<double>::quiet_NaN();
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
