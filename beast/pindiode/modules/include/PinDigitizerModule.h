/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PINDIGITIZERMODULE_H
#define PINDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <beast/pindiode/dataobjects/PindiodeHit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>


namespace Belle2 {
  namespace pindiode {
    /**
     * Pin tube digitizer
     *
     * Creates PinHits from PinSimHits
     *
     */
    class PinDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      PinDigitizerModule();

      /**  */
      virtual ~PinDigitizerModule();

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


    private:
      StoreArray<PindiodeHit> m_pindiodeHit; /** Array for PindiodeHit */

      /** Cremat gain */
      double m_CrematGain;
      /** Work function */
      double m_WorkFunction;
      /** Fano Factor */
      double m_FanoFactor;

    };

  }
}

#endif /* PINDIGITIZERMODULE_H */
