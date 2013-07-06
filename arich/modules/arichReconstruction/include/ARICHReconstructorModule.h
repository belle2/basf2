/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj, Dino Tahirovic                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHRECONSTRUCTORMODULE_H
#define ARICHRECONSTRUCTORMODULE_H

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <arich/modules/arichReconstruction/ARICHReconstruction.h>

#include <string>

namespace Belle2 {
  namespace arich {
    /** This module calculates the values of particles identity likelihood function
     *
     * It reads the tracks and hits information from datastore, for each
     * track it creates internal vector ARICHTrack, calculates
     * values of likelihood function
     * for different particle hypotheses and stores them in datastore.
     */

    class ARICHReconstructorModule : public Module {

    public:

      /*! Constructor.*/
      ARICHReconstructorModule();

      /*! Destructor.*/
      virtual ~ARICHReconstructorModule();

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

      // Input collections
      std::string m_mcColName;         /**< Input collection name */
      std::string m_tracksColName;     /**< MDST tracks */
      std::string m_extHitsColName;    /**< Extrapolated tracks */

      // Output collection
      std::string m_outColName;        /**< Output likelihoods */

      // Other members.
      ARICHReconstruction* m_ana;      /**< Class with reconstruction tools */
      double m_timeCPU;                /**< CPU time.     */
      int    m_nRun;                   /**< Run number.   */
      int    m_nEvent;                 /**< Event number. */
      double m_trackPositionResolution;/**< Track position resolution from tracking. */
      double m_trackAngleResolution;   /**< Track direction resolution from tracking. */
      double m_backgroundLevel;        /**< Photon background level. */
      double m_singleResolution;       /**< Resolution of single photon emission angle. */
      std::vector<double> m_aerogelMerit; /**< Vector of aerogel layer figures of merit. */
      int m_inputTrackType;            /**< Input tracks from the tracking (0) or from MCParticles>AeroHits (1). */
      int    m_debug;                  /**< Debug Level For ARICH, >2 means beamtest*/

      //! Function fills the internal vector ARICHTracks from ext module
      /*!
        \param tracks Internal ARICH vector reference.
        \param hypothesis The hypothesis for the particle.
       */
      void getTracks(std::vector<ARICHTrack>& tracks, Const::ChargedStable hypothesis);

    };

  } // arich namespace
} // Belle2 namespace

#endif // ARICHRECONSTRUCTORMODULE
