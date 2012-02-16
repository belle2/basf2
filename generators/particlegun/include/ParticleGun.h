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
      uniformCosinusDistribution,
      /** Normal distribution, parameters are mean and sigma */
      normalDistribution,
      /** Normal distribution of Pt, parameters are mean and sigma */
      normalPtDistribution,
      /** Fixed Spectra, parameters are pairs of numbers giving weight and value for each spectrum value */
      discreteSpectrum,
    };

    /** Struct to keep all necessary parameters for the particle gun */
    struct Parameters {
      /** Distribution to use for momentum generation */
      Distribution momentumDist;
      /** Distribution to use for azimuth angle generation */
      Distribution phiDist;
      /** Distribution to use for polar angle generation */
      Distribution thetaDist;
      /** Distribution to use for vertex generation */
      Distribution vertexDist;
      /** Number of tracks to generate per event */
      double nTracks;
      /** List of PDG particle codes to pick from when generating particles */
      std::vector<int> pdgCodes;
      /** Parameters for the momentum generation, meaning depends on chosen distribution */
      std::vector<double> momentumParams;
      /** Parameters for the azimuth angle generation, meaning depends on chosen distribution */
      std::vector<double> phiParams;
      /** Parameters for the polar angle generation, meaning depends on chosen distribution */
      std::vector<double> thetaParams;
      /** Parameters for the x vertex generation, meaning depends on chosen distribution */
      std::vector<double> xVertexParams;
      /** Parameters for the y vertex generation, meaning depends on chosen distribution */
      std::vector<double> yVertexParams;
      /** Parameters for the z vertex generation, meaning depends on chosen distribution */
      std::vector<double> zVertexParams;
      /** If false, all particles of one event will have the same vertex, if
       * true the vertex of each particle will be generated independently
       */
      bool independentVertices;
      /** If true, the number of tracks will fluctuate according to Poisson distribution */
      bool varyNumberOfTracks;
    };

    /** Default constructor */
    ParticleGun() {}

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
    double generateValue(Distribution dist, const std::vector<double> params);

    /** All relevant parameters */
    Parameters m_params;
  };

} //end namespace Belle2

#endif //PARTICLEGUN_H
