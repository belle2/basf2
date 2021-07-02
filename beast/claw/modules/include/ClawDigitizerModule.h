/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef CLAWDIGITIZERMODULE_H
#define CLAWDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/claw/dataobjects/ClawHit.h>
#include <vector>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace claw {
    /**
     * Claw tube digitizer
     *
     * Creates ClawHits from ClawSimHits
     *
     */
    class ClawDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      ClawDigitizerModule();

      /**
       * Destructor
       */
      virtual ~ClawDigitizerModule();

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

      StoreArray<ClawHit> m_clawHit; /**< array for ClawHit */

      /** reads data from CLAW.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Time step */
      double m_TimeStep;
      /** Convertor factor keV to MIP */
      double m_C_keV_to_MIP;
      /** Convertor factor MIP to PE */
      std::vector<Double_t> m_C_MIP_to_PE;
      /** Number of CLAW scintillator cell */
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

#endif /* CLAWDIGITIZERMODULE_H */
