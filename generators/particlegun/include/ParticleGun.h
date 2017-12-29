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
#include <mdst/dataobjects/MCParticleGraph.h>

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
    enum EDistribution {
      /** Fixed value, no random generation at all, 1 parameter */
      c_fixedValue,
      /** Uniform distribution, parameters are min and max value */
      c_uniformDistribution,
      /** Uniform distribution of Pt, parameters are min and max value */
      c_uniformPtDistribution,
      /** Uniform distribution of the cosine of the values, parameters are min and max value */
      c_uniformCosDistribution,
      /** Uniform distribution of the logarithm of the values, parameters are min and max value */
      c_uniformLogDistribution,
      /** Uniform distribution of the logarithm of the Pt, parameters are min and max value */
      c_uniformLogPtDistribution,
      /** Normal distribution, parameters are mean and sigma */
      c_normalDistribution,
      /** Normal distribution of Pt, parameters are mean and sigma */
      c_normalPtDistribution,
      /** Normal distribution of the cosinge, parameters are mean and sigma */
      c_normalCosDistribution,
      /** Discrete spectrum, parameters are first the values and then the weights (non-negative) for each value */
      c_discreteSpectrum,
      /** Discrete pt spectrum, parameters are first the values and then the weights (non-negative) for each value */
      c_discretePtSpectrum,
      /** Distribution uniform in the inverse pt distribution, that is uniform in track curvature */
      c_inversePtDistribution,
      /** Distribution given as list of (x,y) points. The Distribution will
       * follow the line connection all points. Parameters are first the x
       * coordinates (sorted) and then the y coordinates (non-negative) */
      c_polylineDistribution,
      /** Same as polylineDistribution but for the transverse momentum */
      c_polylinePtDistribution,
      /** Same as polylineDistribution but for the cosine of the angle */
      c_polylineCosDistribution
    };

    /** Struct to keep all necessary parameters for the particle gun */
    struct Parameters {
      /** Distribution to use for momentum generation */
      EDistribution momentumDist = c_fixedValue;
      /** Distribution to use for azimuth angle generation */
      EDistribution phiDist = c_fixedValue;
      /** Distribution to use for polar angle generation */
      EDistribution thetaDist = c_fixedValue;
      /** Distribution to use for x vertex generation */
      EDistribution xVertexDist = c_fixedValue;
      /** Distribution to use for x vertex generation */
      EDistribution yVertexDist = c_fixedValue;
      /** Distribution to use for x vertex generation */
      EDistribution zVertexDist = c_fixedValue;
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
      /** generates particle momentum according to the specified
       * distribution and assigns this momentum to all particles generated
       * for one event */
      bool fixedMomentumPerEvent = false;
      /** Time offset from 0 */
      double timeOffset;

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
    double generateValue(EDistribution dist, const std::vector<double>& params);

    /** All relevant parameters */
    Parameters m_params;
  };

} //end namespace Belle2

#endif //PARTICLEGUN_H
