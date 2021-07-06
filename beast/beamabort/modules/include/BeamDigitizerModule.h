/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BEAMDIGITIZERMODULE_H
#define BEAMDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/beamabort/dataobjects/BeamabortHit.h>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace beamabort {
    /**
     * Beam tube digitizer
     *
     * Creates BeamHits from BeamSimHits
     *
     */
    class BeamDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      BeamDigitizerModule();

      /**
       * Destructor
       */
      virtual ~BeamDigitizerModule();

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

      StoreArray<BeamabortHit> m_beamabortHit; /**< Array for BemaabortHit */

      /** Work function */
      double m_WorkFunction;
      /** Fano Factor */
      double m_FanoFactor;
    };

  }
}

#endif /* BEAMDIGITIZERMODULE_H */
