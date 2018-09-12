/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HE3DIGITIZERMODULE_H
#define HE3DIGITIZERMODULE_H

#include <beast/he3tube/dataobjects/He3tubeSimHit.h>

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <beast/he3tube/dataobjects/He3tubeHit.h>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>

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
      /**  */
      virtual ~He3DigitizerModule();
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
      StoreArray<He3tubeHit> m_he3tubeHit; /** Array for He3tubeHit */

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
