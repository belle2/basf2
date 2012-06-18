/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPDIGIMODULE_H
#define TOPDIGIMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>

namespace Belle2 {
  namespace TOP {
    //! TOP digitizer module.
    /*
     * This module takes hits form G4 simulation (TOPSimHit),
     * applies q.e. of PMTs, TTS, T0jitter and do spatial and time digitization
     * output to TOPDigiHit.
     */
    class TOPDigiModule : public Module {

    public:

      //! Constructor.
      TOPDigiModule();

      //! Destructor.
      virtual ~TOPDigiModule();

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

      std::string m_inColName;    /**< Input collection name */
      std::string m_outColName;   /**< Output collection name */
      double m_photonFraction;    /**< Fraction of Cer. photons propagated in FullSim */
      double m_T0jitter;          /**< r.m.s of T0 jitter */
      double m_ELjitter;          /**< r.m.s of electronics jitter */

      //! Geometry parameters reading object
      TOPGeometryPar* m_topgp;

      /*! Apply quantum times collection efficiency via hit or miss method
       * @param energy energy of photon in eV
       * @return true if photon is detected
       */
      bool DetectorQE(double energy);

      //! Returns random number according to TTS distribution
      double PMT_TTS();

    };

  } // top namespace
} // Belle2 namespace

#endif // TOPDIGIMODULE_H
