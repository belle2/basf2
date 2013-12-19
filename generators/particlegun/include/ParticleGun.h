/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEGUN_H
#define PARTICLEGUN_H

#include <framework/core/FrameworkExceptions.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include <vector>
#include <TRandom3.h>

namespace Belle2 {

  /**
   * Class to generate tracks in the particle gun and store them in a MCParticle graph.
   * The class supports multiple tracks per event, different PDGcodes
   * and the range for track momenta, track angle and track origin is configurable.
   */
  class ParticleGun {
  public:
    /** enum containing all known distributions available for generation of values */
    enum Distribution {
      /** Fixed value, no random generation at all, 1 parameter */
      fixedValue,
      /** Uniform distribution, parameters are min and max value */
      uniformDistribution,
      /** Uniform distribution of Pt, parameters are min and max value */
      uniformPtDistribution,
      /** Uniform distribution of the cosine of the values, parameters are min and max value */
      uniformCosDistribution,
      /** Normal distribution, parameters are mean and sigma */
      normalDistribution,
      /** Normal distribution of Pt, parameters are mean and sigma */
      normalPtDistribution,
      /** Normal distribution of the cosinge, parameters are mean and sigma */
      normalCosDistribution,
      /** Discrete spectrum, parameters are first the values and then the weights (non-negative) for each value */
      discreteSpectrum,
      /** Distribution uniform in the inverse pt distribution, that is uniform in track curvature */
      inversePtDistribution,
      /** Distribution given as list of (x,y) points. The Distribution will
       * follow the line connection all points. Parameters are first the x
       * coordinates (sorted) and then the y coordinates (non-negative) */
      polylineDistribution,
      /** Same as polylineDistribution but for the transverse momentum */
      polylinePtDistribution,
      /** Same as polylineDistribution but for the cosine of the angle */
      polylineCosDistribution
    };

    /** Struct to keep all necessary parameters for the particle gun */
    struct Parameters {
      /** Distribution to use for momentum generation */
      Distribution momentumDist = fixedValue;
      /** Distribution to use for azimuth angle generation */
      Distribution phiDist = fixedValue;
      /** Distribution to use for polar angle generation */
      Distribution thetaDist = fixedValue;
      /** Distribution to use for x vertex generation */
      Distribution xVertexDist = fixedValue;
      /** Distribution to use for x vertex generation */
      Distribution yVertexDist = fixedValue;
      /** Distribution to use for x vertex generation */
      Distribution zVertexDist = fixedValue;
      /** Number of tracks to generate per event */
      double nTracks;
      /** List of PDG particle codes to pick from when generating particles */
      std::vector<int> pdgCodes = {};
      /** Parameters for the momentum generation, meaning depends on chosen distribution */
      std::vector<double> momentumParams = {0};
      /** Parameters for the azimuth angle generation, meaning depends on chosen distribution */
      std::vector<double> phiParams = {0};
      /** Parameters for the polar angle generation, meaning depends on chosen distribution */
      std::vector<double> thetaParams = {0};
      /** Parameters for the x vertex generation, meaning depends on chosen distribution */
      std::vector<double> xVertexParams = {0};
      /** Parameters for the y vertex generation, meaning depends on chosen distribution */
      std::vector<double> yVertexParams = {0};
      /** Parameters for the z vertex generation, meaning depends on chosen distribution */
      std::vector<double> zVertexParams = {0};
      /** If false, all particles of one event will have the same vertex, if
       * true the vertex of each particle will be generated independently
       */
      bool independentVertices = false;
      /** If true, the number of tracks per event will fluctuate according to
       * Poisson distribution */
      bool varyNumberOfTracks = true;
    };

    /** Default constructor */
    ParticleGun(): m_params() {}

    /** Default destructor */
    ~ParticleGun() {}

    /**
     * Generate the next event and store the result in the given MCParticle graph.
     * @return true if the event was generated.
     */
    bool generateEvent(MCParticleGraph& graph);

    /** Set the parameters for generating the Particles */
    bool setParameters(const Parameters& parameters);

  protected:
    /** Generate a value according to the given distribution with the given parameters */
    double generateValue(Distribution dist, const std::vector<double>& params);

    /** All relevant parameters */
    Parameters m_params;
  };

} //end namespace Belle2

#endif //PARTICLEGUN_H
