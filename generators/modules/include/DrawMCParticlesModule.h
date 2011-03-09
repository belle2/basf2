/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DRAWMCPARTICLESMODULE_H
#define DRAWMCPARTICLESMODULE_H

#include <framework/core/Module.h>
#include <generators/dataobjects/MCParticle.h>

namespace Belle2 {

  namespace Generators {

    /** The DrawMCParticles module.
     * Draws the content of the MCParticle collection into the gGeoManager geometry.
     * For each MCParticle an arrow is drawn at its vertex position and pointing into
     * its momentum direction.
     */
    class DrawMCParticlesModule : public Module {

    public:

      /** Constructor. */
      DrawMCParticlesModule();

      /** Destructor. */
      virtual ~DrawMCParticlesModule() {}

      /** Method is called for each event. */
      virtual void event();


    protected:

      /** Loops recursively over the MCParticle list and draws each particle.
       * @param mc Reference to the MCParticle which should be drawn and whose daughters should be visited.
       */
      void drawTree(const MCParticle &mc);

      std::string m_particleList; /**< The name of the MCParticle collection. */
      std::vector<bool> m_seen;   /**< Tag the particles which were already visited using their index. */
      bool m_onlyPrimaries;       /**< Print only primary particles. */
    };

  }//end namespace Generators

} // end namespace Belle2

#endif /* DRAWMCPARTICLESMODULE_H */
