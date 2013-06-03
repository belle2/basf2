/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPDIGITIZERMODULE_H
#define TOPDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>

namespace Belle2 {
  namespace TOP {
    //! TOP digitizer module.
    /*
     * This module takes hits form G4 simulation (TOPSimHit),
     * applies TTS, T0jitter and do spatial and time digitization
     * (QE moved to the simulation: applied in SensitiveBar, SensitivePMT)
     * output to TOPDigiHit.
     */
    class TOPDigitizerModule : public Module {

    public:

      //! Constructor.
      TOPDigitizerModule();

      //! Destructor.
      virtual ~TOPDigitizerModule();

      /**
       * Initialize the Module.
       * This method is called at the beginning of data processing.
       */
      virtual void initialize();

      /**
       * Called when entering a new run.
       * Set run dependent things like run header parameters, alignment, etc.
       */
      virtual void beginRun();

      /**
       * Event processor.
       * Convert TOPSimHits to TOPDigiHits.
       */
      virtual void event();

      /**
       * End-of-run action.
       * Save run-related stuff, such as statistics.
       */
      virtual void endRun();

      /**
       * Termination action.
       * Clean-up, close files, summarize statistics, etc.
       */
      virtual void terminate();

      /**
       * Prints module parameters.
       */
      void printModuleParams() const;

    private:

      std::string m_inputSimHits;    /**< Input collection name */
      std::string m_outputDigits;    /**< Output collection name */
      double m_timeZeroJitter;       /**< r.m.s of T0 jitter */
      double m_electronicJitter;     /**< r.m.s of electronic jitter */

      //! Geometry parameters reading object
      TOPGeometryPar* m_topgp;

      //! Returns random number according to TTS distribution
      double PMT_TTS();

    };

  } // top namespace
} // Belle2 namespace

#endif // TOPDIGITIZERMODULE_H
