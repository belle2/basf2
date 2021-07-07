/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef PLUMEDIGITIZERMODULE_H
#define PLUMEDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <beast/plume/dataobjects/PlumeHit.h>
#include <framework/datastore/StoreArray.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

namespace Belle2 {
  namespace plume {
    /**
     * PLUME digitizer
     *
     * Creates PlumeHits from PlumeSimHits
     *
     */
    class PlumeDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      PlumeDigitizerModule();

      /**
       * Destructor
       */
      virtual ~PlumeDigitizerModule();

      /**
       * Initialize the Module.
       * This method is called at the beginning of data processing.
       */
      virtual void initialize() override;

      /**
       * Called when entering a new run.
       * Set run dependent things like run header parameters, alignment, etc.
       */
      virtual void beginRun() override;

      /**
       * Event processor.
       */
      virtual void event() override;

      /**
       * End-of-run action.
       * Save run-related stuff, such as statistics.
       */
      virtual void endRun() override;

      /**
       * Termination action.
       * Clean-up, close files, summarize statistics, etc.
       */
      virtual void terminate() override;

      /** set the parameters of the response model */
      virtual void setResponseModel();

      /** reads data from PLUME.xml: ladder location, pixel pitch, epitaxial thickness, integration time */
      virtual void getXMLData();

    private:
      /** Event number */
      int Event = 0;
      /** number of pixels */
      int m_nofPixels;
      /** local position u */
      float m_posmm_u;
      /** local position v */
      float m_posmm_v;
      /** G4 position x */
      float m_posmm_x;
      /** G4 position y */
      float m_posmm_y;
      /** G4 position z */
      float m_posmm_z;

      /** collection of PlumeHit saved in the datastore by the module*/
      StoreArray<PlumeHit> m_plumeHits; /**< PLUME hits */
      StoreArray<MCParticle> m_particles; /**< PLUME particles */
      StoreArray<PlumeSimHit> m_plumeSimHits; /**< PLUME SimHits */
    };

  }
}

#endif /* PLUMEDIGITIZERMODULE_H */
