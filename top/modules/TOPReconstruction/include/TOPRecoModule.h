/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPRECOMODULE_H
#define TOPRECOMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>

namespace Belle2 {
  namespace TOP {
    //! TOP reconstruction module.
    /*!
    */
    class TOPRecoModule : public Module {

    public:


      //! Constructor.
      TOPRecoModule();

      //! Destructor.
      virtual ~TOPRecoModule();

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
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */

      //! Parameter reading object
      TOPGeometryPar* m_topgp;

      //! TOP configure function
      void TOPconfigure();


    };

  } // top namespace
} // Belle2 namespace

#endif // TOPRECOMODULE_H
