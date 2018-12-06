/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong, Jerome Baudot                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PLUMEDIGITIZERMODULE_H
#define PLUMEDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <beast/plume/dataobjects/PlumeHit.h>
#include <framework/datastore/StoreArray.h>
#include <beast/plume/dataobjects/PlumeSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <string>


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

      /**  */
      virtual ~PlumeDigitizerModule();

      /**  */
      virtual void initialize() override;

      /**  */
      virtual void beginRun() override;

      /**  */
      virtual void event() override;

      /**  */
      virtual void endRun() override;

      /**  */
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
      StoreArray<PlumeHit> m_plumeHits;
      StoreArray<MCParticle> m_particles;
      StoreArray<PlumeSimHit> m_plumeSimHits;
    };

  }
}

#endif /* PLUMEDIGITIZERMODULE_H */
