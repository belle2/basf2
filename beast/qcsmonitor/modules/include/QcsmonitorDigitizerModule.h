/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef QCSMONITORDIGITIZERMODULE_H
#define QCSMONITORDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <beast/qcsmonitor/dataobjects/QcsmonitorHit.h>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace qcsmonitor {
    /**
     * Qcsmonitor tube digitizer
     *
     * Creates QcsmonitorHits from QcsmonitorSimHits
     *
     */
    class QcsmonitorDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      QcsmonitorDigitizerModule();

      /**
       * Destructor
       */
      virtual ~QcsmonitorDigitizerModule();

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
      StoreArray<QcsmonitorHit> m_qcsmonitorHit; /**< Array for QcsmonitorHit */

      /** reads data from QCSMONITOR.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** Time step */
      double m_TimeStep;
      /** Converter factor keV to MIP */
      double m_C_keV_to_MIP;
      /** Converter factor MIP to PE */
      double m_C_MIP_to_PE;
      /** Number of QCSMONITOR scintillator cell */
      int m_ScintCell;
      /** Lower time limit */
      double m_MinTime;
      /** Upper time limit */
      double m_MaxTime;
      /** Energy threshold */
      double m_MIPthres;
      /** Event MIP array*/
      double hitsarrayinMIP[1000][100];

    };

  }
}

#endif /* QCSMONITORDIGITIZERMODULE_H */
