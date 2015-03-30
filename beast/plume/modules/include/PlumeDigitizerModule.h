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
      virtual void initialize();

      /**  */
      virtual void beginRun();

      /**  */
      virtual void event();

      /**  */
      virtual void endRun();
      /**  */
      virtual void terminate();

      /** set the parameters of the response model */
      virtual void setResponseModel();

      /** reads data from PLUME.xml: ladder location, pixel pitch, epitaxial thickness, integration time */
      virtual void getXMLData();

    private:
      int Event = 0;

      int m_nofPixels;
      float m_posmm_u;
      float m_posmm_v;
      float m_posmm_x;
      float m_posmm_y;
      float m_posmm_z;

    };

  }
}

#endif /* PLUMEDIGITIZERMODULE_H */
