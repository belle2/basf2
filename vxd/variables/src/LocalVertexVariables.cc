/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/utility/ReferenceFrame.h>
#include <analysis/VariableManager/Manager.h>

#include <Math/Vector3D.h>

#include <mdst/dataobjects/MCParticle.h>

#include <vxd/geometry/GeoCache.h>

#include <tuple>

namespace Belle2 {
  class Particle;

  namespace Variable {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();

    // MC Local coordinates and sensor ID from getDecayVertex global coordinates

    std::tuple<ROOT::Math::XYZVector, int, int, int> getmcLocalCoordinates(const Particle* part)
    {
      VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      auto* mcparticle = part->getMCParticle();
      if (!mcparticle)
        return std::make_tuple(ROOT::Math::XYZVector(realNaN, realNaN, realNaN), 0, 0, 0);

      const auto& mcglobal = mcparticle->getDecayVertex();

      for (const auto& layer : geo.getLayers()) {
        for (const auto& ladder : geo.getLadders(layer)) {
          for (const auto& sensor : geo.getSensors(ladder)) {
            const auto& sInfo = VXD::GeoCache::get(sensor);
            const auto& mclocal = sInfo.pointToLocal(mcglobal, true);
            if (sInfo.inside(mclocal))
              return std::make_tuple(mclocal, sensor.getLayerNumber(), sensor.getLadderNumber(), sensor.getSensorNumber());
          }
        }
      }
      return std::make_tuple(ROOT::Math::XYZVector(realNaN, realNaN, realNaN), 0, 0, 0);
    }

    double mcDecayVertexU(const Particle* part)
    {
      return std::get<0>(getmcLocalCoordinates(part)).X();
    }

    double mcDecayVertexV(const Particle* part)
    {
      return std::get<0>(getmcLocalCoordinates(part)).Y();
    }

    double mcDecayVertexW(const Particle* part)
    {
      return std::get<0>(getmcLocalCoordinates(part)).Z();
    }
    double mcDecayVertexLayer(const Particle* part)
    {
      return std::get<1>(getmcLocalCoordinates(part));
    }

    double mcDecayVertexLadder(const Particle* part)
    {
      return std::get<2>(getmcLocalCoordinates(part));
    }

    double mcDecayVertexSensor(const Particle* part)
    {
      return std::get<3>(getmcLocalCoordinates(part));
    }

    // Local coordinates and sensor ID from getVertex global coordinates

    std::tuple<ROOT::Math::XYZVector, int, int, int> getLocalCoordinates(const Particle* part)
    {
      VXD::GeoCache& geo = VXD::GeoCache::getInstance();
      const auto& frame = ReferenceFrame::GetCurrent();
      const auto& global = frame.getVertex(part);

      for (const auto& layer : geo.getLayers()) {
        for (const auto& ladder : geo.getLadders(layer)) {
          for (const auto& sensor : geo.getSensors(ladder)) {

            const auto& sInfo = VXD::GeoCache::get(sensor);
            const auto& local = sInfo.pointToLocal(global, true);
            if (sInfo.inside(local.X(), local.Y(), 0.1, 0.1)) {
              if (abs(local.Z()) < 0.1) {
                return std::make_tuple(local, sensor.getLayerNumber(), sensor.getLadderNumber(), sensor.getSensorNumber());
              } else {
                ROOT::Math::XYZVector localz{local.X(), local.Y(), abs(local.Z()) - 0.1};
                if (sInfo.inside(localz))
                  return std::make_tuple(local, sensor.getLayerNumber(), sensor.getLadderNumber(), sensor.getSensorNumber());
              }
            }

          }
        }
      }
      return std::make_tuple(ROOT::Math::XYZVector(realNaN, realNaN, realNaN), 0, 0, 0);
    }

    double particleU(const Particle* part)
    {
      return std::get<0>(getLocalCoordinates(part)).X();
    }

    double particleV(const Particle* part)
    {
      return std::get<0>(getLocalCoordinates(part)).Y();
    }

    double particleW(const Particle* part)
    {
      return std::get<0>(getLocalCoordinates(part)).Z();
    }
    double particleLayer(const Particle* part)
    {
      return std::get<1>(getLocalCoordinates(part));
    }

    double particleLadder(const Particle* part)
    {
      return std::get<2>(getLocalCoordinates(part));
    }

    double particleSensor(const Particle* part)
    {
      return std::get<3>(getLocalCoordinates(part));
    }

    VARIABLE_GROUP("Local Vertex Information");
    // Generated vertex information
    REGISTER_VARIABLE("mcDecayVertexU", mcDecayVertexU,
                      "Returns the U position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexV", mcDecayVertexV,
                      "Returns the V position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexW", mcDecayVertexW,
                      "Returns the W position of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexLayer", mcDecayVertexLayer,
                      "Returns the Layer ID of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexLadder", mcDecayVertexLadder,
                      "Returns the ladder ID of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("mcDecayVertexSensor", mcDecayVertexSensor,
                      "Returns the sensor ID of the decay vertex of the matched generated particle. Returns nan if the particle has no matched generated particle.");
    REGISTER_VARIABLE("u", particleU,
                      "u local sensor coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("v", particleV,
                      "V local sensor coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("w", particleW,
                      "w local sensor coordinate of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("layer", particleLayer,
                      "layer identification of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("ladder", particleLadder,
                      "layer identification of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");
    REGISTER_VARIABLE("sensor", particleSensor,
                      "sensor identification of vertex in case of composite particle, or point of closest approach (POCA) in case of a track");

  }
}
