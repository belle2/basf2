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

#include <framework/core/Module.h>
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

      /** reads data from HE3TUBE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();
      /** filename of Garfield datafile */
      /** number of detectors. Read from the gearbox*/
      int numOfTubes = 0;

      /** Event counter */
      int Event = 0;
      /** Conversion to ADC counts, set in steering file*/
      double m_ConversionFactor;
      /** ionization energy of He3 */
      double m_Workfct = 35.075;
      /** fano factor */
      double m_Fanofac = 220;

    };

  }
}

#endif /* HE3DIGITIZERMODULE_H */
