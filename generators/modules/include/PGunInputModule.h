/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PGUNINPUTMODULE_H
#define PGUNINPUTMODULE_H

#include <framework/core/Module.h>

#include <string>
#include <generators/dataobjects/MCParticle.h>
#include <generators/dataobjects/MCParticleGraph.h>
#include <generators/pGun/ParticleGun.h>
#include <vector>

namespace Belle2 {

  namespace Generators {

    /** The pGunInput module.
     * Generate tracks with the particle gun and store them
     * into the MCParticle class.
     */
    class PGunInputModule : public Module {

    public:

      /**
       * Constructor.
       * Sets the module parameters.
       */
      PGunInputModule();

      /** Destructor. */
      virtual ~PGunInputModule() {}

      /** Initializes the module. */
      virtual void initialize();

      /** Method is called for each event. */
      virtual void event();

    protected:

      ParticleGun m_pgun;             /**< An instance of the particle gun itself. */
      MCParticleGraph mpg;            /**< An instance of the MCParticle graph. */
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
      int m_genMom;          /**< Option to set the distribution function for the momentum */
      int m_genVert;          /**< Option to set the distribution function for the vertex */
      int m_genAngle;        /**< Option to set the distribution function for the angles */
      int m_randomseed;        /**< setting the random seed for the particle gun. This will disappear, once the central random generator is running **/



      /** the list of particle types to be produced */
      std::vector <int>  m_PIDcodes;
    };

  }//end namespace Generators

} // end namespace Belle2

#endif // PGUNINPUTMODULE_H
