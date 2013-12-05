/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHDIGITIZERMODULE_H
#define ARICHDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <arich/geometry/ARICHGeometryPar.h>

#include <string>

namespace Belle2 {
  namespace arich {
    /** ARICH digitizer module.
      *
      * This module takes the hits form GEANT4 simulation (ARICHSimHit),
      * applies q.e. of HAPDs, calculates and saves hit channel numbers (ARICHDigit).
      * If the channel has multiple hits, only one is saved.
      */
    class ARICHDigitizerModule : public Module {

    public:


      //! Constructor.
      ARICHDigitizerModule();

      //! Destructor.
      virtual ~ARICHDigitizerModule();

      /**
       * Initialize the Module.
       *
       * This method is called at the beginning of data processing.
       */
      virtual void initialize();

      /**
       * Called when entering a new run.
       *
       * Set run dependent things like run header parameters, alignment, etc.
       */
      virtual void beginRun();

      /**
       * Event processor.
       *
       * Convert ARICHSimHits of the event to arichDigits.
       */
      virtual void event();

      /**
       * End-of-run action.
       *
       * Save run-related stuff, such as statistics.
       */
      virtual void endRun();

      /**
       * Termination action.
       *
       * Clean-up, close files, summarize statistics, etc.
       */
      virtual void terminate();

      /**
       *Prints module parameters.
       */
      void printModuleParams() const;

    private:

      std::string m_inColName;         /**< Input collection name from simulation */
      std::string m_outColName;        /**< Output collection name: digitized photons hits */

      /* Other members.*/
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */

      ARICHGeometryPar* m_arichgp;    /**< Geometry parameters of ARICH. */

      //! Returns q.e. of detector at given photon energy
      /*!
      \param energy energy at which q.e. is returned
      */
      double QESuperBialkali(double energy);

      //! Apply q.e., returns 1 if photon is detected and 0 if not.
      /*!
      \param energy energy of photon
      */
      int DetectorQE(double energy);

    };

  } // arich namespace
} // Belle2 namespace

#endif // ARICHDIGITIZERMODULE_H
