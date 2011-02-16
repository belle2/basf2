/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
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

  namespace Generators { //namespace Generators

    /** This enum keeps track of the distributions that are supported for the random number generation. */
    enum EPGUNgenOpt {
      uniform,
      gauss,
      none
    };

    /**
     * Class to generate tracks in the particle gun and store them in a MCParticle graph.
     * The class supports multiple tracks per event, different PIDcodes
     * and the range for track momenta, track angle and track origin is configurable.
     */
    class ParticleGun {

    public:

      //Define exceptions



      /**
       * Constructor.
       */
      ParticleGun() {

        // m_grand.SetSeed(m_randomseed); /** For the time being, always the same fixed seed is used. This will be changed when the random generator becomes a global object from the framework. */

      };

      /**
       * Destructor.
       */
      ~ParticleGun() {}

      /**
       * This function sets the random seed for the particle gun
       * random generator. The seed is taken from the steering file.
       * This function will become obsolete, when a global random generator
       * for all generation and simulation processes is implemented.
       */
      void SetRandomSeed(int seed) {

        m_grand.SetSeed(seed);

      }


      /**
       * Generate the next event and store the result in the given MCParticle graph.
       * @return true if the event was generated.
       */
      bool generateEvent(MCParticleGraph &graph);

      int m_ntracks;                  /**< The number of particles to be produced per event */
      double m_p_par1;                /**< The first parameter for momentum generation */
      double m_p_par2;                /**< The second parameter for momentum generation */
      double m_ph_par1;               /**< The first parameter for phi generation */
      double m_ph_par2;               /**< The second parameter for phi generation */
      double m_th_par1;               /**< The first parameter for theta generation */
      double m_th_par2;               /**< The second parameter for theta generation */
      double m_x_par1;                /**< The first parameter for vertex x coordinate generation */
      double m_x_par2;                /**< The second parameter for vertex x coordinate generation */
      double m_y_par1;                /**< The first parameter for vertex y coordinate generation */
      double m_y_par2;                /**< The second parameter for vertex y coordinate generation */
      double m_z_par1;                /**< The first parameter for vertex z coordinate generation */
      double m_z_par2;                /**< The second parameter for vertex z coordinate generation */
      EPGUNgenOpt m_genMom;            /**< Option to set the distribution function for the momentum */
      EPGUNgenOpt m_genVert;           /**< Option to set the distribution function for the vertex */
      EPGUNgenOpt m_genAngle;          /**< Option to set the distribution function for the angles */

      /** the list of particle types to be produced */
      std::vector <int>  m_PIDcodes;


    protected:

      TRandom3 m_grand;          /**< Instance of the random generator **** This will be moved to the framework eventually. */
    };

  }//end namespace Generators
} //end namespace Belle2

#endif //PARTICLEGUN_H



