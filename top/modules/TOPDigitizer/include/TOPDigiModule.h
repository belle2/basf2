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

#include <TRandom1.h>



namespace Belle2 {
  namespace TOP {
    //! TOP digitizer module.
    /*!
      This module takes the hits form G4 simulation (TOPSimHit), applies q.e. of PMTs, calculates and saves hit channel numbers (TOPHit).
      No time spread and double hits are used
    */
    class TOPDigiModule : public Module {

    public:


      //! Constructor.
      TOPDigiModule();

      //! Destructor.
      virtual ~TOPDigiModule();

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
       * Convert TOPSimHits of the event to TOPHits.
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

      std::string m_inColName;         /**< Input collection name */
      std::string m_outColName;        /**< Output collection name */

      /* Other members.*/
      TRandom1* m_random;              /**< Random number generator.*/
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */

      //!Parameter reading object
      TOPGeometryPar* m_topgp;
      //! Returns q.e. of detector at given photon energy
      /*!
        \param energy energy at which q.e. is returned
       */
      double QESuperBialkali(double energy);

      //! Returns q.e. of detector at given photon energy
      /*!
       \param energy energy at which q.e. is returned
       */
      double QEMultiAlkali(double energy);


      //! Apply q.e., returns trure if photon is detected and false if not.
      /*!
        \param energy energy of photon
       */
      bool DetectorQE(double energy);

      //! Returns gausian distribuded random number
      /*!
       \param mean
       \param sigma
      */
      double Gaus(double mean, double sigma);

      //! Returns gausian distribuded random number
      double PMT_TTS();


    };

  } // top namespace
} // Belle2 namespace

#endif // TOPDIGIMODULE_H
