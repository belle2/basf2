/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BEAMDIGITIZERMODULE_H
#define BEAMDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>

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

      /**  */
      virtual ~BeamDigitizerModule();

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

      /** reads data from BEAMABORT.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** number of detectors. Read from BEAMABORT.xml*/
      int nBEAM = 0;
      /** BEAM coordinate */
      std::vector<TVector3> BEAMCenter;
      /** Event counter */
      int Event = 0;

    };

  }
}

#endif /* BEAMDIGITIZERMODULE_H */
