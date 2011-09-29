/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRECMODULE_H
#define ARICHRECMODULE_H

#include <framework/core/Module.h>

#include <arich/modules/arichReconstruction/ARICHReconstruction.h>

#include <string>

namespace Belle2 {
  namespace arich {
    /**
     * This module calculates the values of particles identity likelihood function
     *
     * It reads the tracks and hits information from datastore, for each
     * track it creates ARICHTrack, calculates values of likelihood function
     * for different particle hypotheses and stores them in datastore.
     */

    class ARICHRecModule : public Module {

    public:


      /*! Constructor.*/
      ARICHRecModule();

      /*! Destructor.*/
      virtual ~ARICHRecModule();

      /**
       * Initialize the Module.
       *
       */
      virtual void initialize();

      /**
       * Called when entering a new run.
       *
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

    protected:

      /*! Print module parameters.*/
      void printModuleParams() const;

    private:

      std::string m_inColName;         /**< Input collection name */
      std::string m_outColName;        /**< Output collection name */

      /** Other members.*/
      ARICHReconstruction* m_ana;      /**< Class with reconstruction tools */
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */
      double m_trackPosRes;            /**< Track position resolution from tracking. */
      double m_trackAngRes;            /**< Track direction resolution from tracking. */
      double m_bkgLevel;               /**< Photon background level. */
      double m_singleRes;              /**< Resolution of single photon emission angle. */
      std::vector<double> m_aeroMerit; /**< Vector of aerogel layer figures of merit. */
    };

  } // arich namespace
} // Belle2 namespace

#endif // ARICHRECMODULE
