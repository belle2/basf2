/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLEGUN_H
#define PARTICLEGUN_H

#include <framework/core/FrameworkExceptions.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include <string>
#include <fstream>
#include <vector>

#include <TRandom3.h>

namespace Belle2 {//namespace Belle2


  /**
   * Class to generate tracks in the particle gun and store them in a MCParticle graph.
   * The class supports multiple tracks per event, different PIDcodes
   * and the range for track momenta, track angle and track origin is configurable.
   */
  class OldParticleGun {

  public:

    /** This enum keeps track of the distributions that are supported for the random number generation. */
    enum EgenerationOption {
      c_uniform,
      c_gauss,
      c_none
    };



    //Define exceptions



    /**
     * Constructor.
     */
    OldParticleGun();

    /**
     * Destructor.
     */
    ~OldParticleGun() {}

    /**
     * This function sets the random seed for the particle gun
     * random generator. The seed is taken from the steering file.
     * This function will become obsolete, when a global random generator
     * for all generation and simulation processes is implemented.
     */
    void SetRandomSeed(int seed) {

      m_gRand.SetSeed(seed);

    }


    /**
     * Generate the next event and store the result in the given MCParticle graph.
     * @return true if the event was generated.
     */
    bool generateEvent(MCParticleGraph &graph);

    int m_nTracks;                  /**< The number of particles to be produced per event */
    double m_pPar1;                /**< The first parameter for momentum generation */
    double m_pPar2;                /**< The second parameter for momentum generation */
    double m_phiPar1;               /**< The first parameter for phi generation */
    double m_phiPar2;               /**< The second parameter for phi generation */
    double m_thetaPar1;               /**< The first parameter for theta generation */
    double m_thetaPar2;               /**< The second parameter for theta generation */
    double m_xVertexPar1;                /**< The first parameter for vertex x coordinate generation */
    double m_xVertexPar2;                /**< The second parameter for vertex x coordinate generation */
    double m_yVertexPar1;                /**< The first parameter for vertex y coordinate generation */
    double m_yVertexPar2;                /**< The second parameter for vertex y coordinate generation */
    double m_zVertexPar1;                /**< The first parameter for vertex z coordinate generation */
    double m_zVertexPar2;                /**< The second parameter for vertex z coordinate generation */
    EgenerationOption m_genMom;            /**< Option to set the distribution function for the momentum */
    EgenerationOption m_genVert;           /**< Option to set the distribution function for the vertex */
    EgenerationOption m_genAngle;          /**< Option to set the distribution function for the angles */
    bool m_fixedPt;                      /**< Option to choose whether Pt or P should be fixed in the generation. */

    /** the list of particle types to be produced */
    std::vector <int>  m_PIDcodes;


  protected:

    TRandom3 m_gRand;          /**< Instance of the random generator **** This will be moved to the framework eventually. */
  };

} //end namespace Belle2

#endif //PARTICLEGUN_H



