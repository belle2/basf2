/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef HE3DIGITIZERMODULE_H
#define HE3DIGITIZERMODULE_H

#include <beast/he3tube/dataobjects/He3tubeSimHit.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/he3tube/dataobjects/He3tubeHit.h>

namespace Belle2 {
  namespace he3tube {
    /**
     * He3 tube digitizer
     *
     * Creates He3Hits from He3SimHits
     *
     */
    class He3DigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      He3DigitizerModule();

      /**
       * Destructor
       */
      virtual ~He3DigitizerModule();

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
      StoreArray<He3tubeHit> m_he3tubeHit; /**< Array for He3tubeHit */

      /** reads data from HE3TUBE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();
      /** number of detectors. Read from the gearbox*/
      int numOfTubes = 0;
      /** Whether or not mcparticle array exists */
      bool m_mcpExist = false;
      /** Event counter */
      int Event = 0;
      /** Conversion to ADC counts, set in steering file*/
      double m_ConversionFactor;
      /** ionization energy of He3 */
      //double m_Workfct = 35.075;
      double m_Workfct = 24.6;
      /** fano factor */
      double m_Fanofac = 220;
      /** Process the he3tube simhits */
      void ProcessHit(He3tubeSimHit* aHit, double* lowTime, double* edepDet, double* NbEle_tot);



    };

  }
}

#endif /* HE3DIGITIZERMODULE_H */
