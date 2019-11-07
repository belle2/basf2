/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CLAWSDIGITIZERMODULE_H
#define CLAWSDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/claws/dataobjects/ClawsHit.h>
#include <vector>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace claws {
    /**
     * Claws tube digitizer
     *
     * Creates ClawsHits from ClawsSimHits
     *
     */
    class ClawsDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      ClawsDigitizerModule();

      /**  */
      virtual ~ClawsDigitizerModule();

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


    private:
      StoreArray<ClawsHit> m_clawsHit; /** Array for ClawsHit */

      /** reads data from CLAWS.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Time step */
      double m_TimeStep;
      /** Convertor factor keV to MIP */
      double m_C_keV_to_MIP;
      /** Convertor factor MIP to PE */
      std::vector<Double_t> m_C_MIP_to_PE;
      /** Number of CLAWS scintillator cell */
      int m_ScintCell;
      /** Lower time limit */
      double m_MinTime;
      /** Upper time limit */
      double m_MaxTime;
      /** Energy threshold */
      double m_PEthres;
      /** Event PE array*/
      //double hitsarrayinPE[1000][100];

    };

  }
}

#endif /* CLAWSDIGITIZERMODULE_H */
