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
#include <framework/datastore/StoreArray.h>

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

      /**
       * Destructor
       */
      virtual ~PinDigitizerModule();

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


    private:
      StoreArray<PindiodeHit> m_pindiodeHit; /**< Array for PindiodeHit */

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
