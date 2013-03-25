/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPRECONSTRUCTORMODULE_H
#define TOPRECONSTRUCTORMODULE_H

#include <framework/core/Module.h>
#include <top/geometry/TOPGeometryPar.h>
#include <string>

#include <GFTrack.h>
#include <generators/dataobjects/MCParticle.h>
#include "top/modules/TOPReconstruction/TOPtrack.h"
#include <framework/gearbox/Const.h>


namespace Belle2 {
  namespace TOP {

    /**
     * TOP reconstruction module.
     */
    class TOPReconstructorModule : public Module {

    public:

      /**
       * Constructor
       */
      TOPReconstructorModule();

      /**
       * Destructor
       */
      virtual ~TOPReconstructorModule();

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
       * Prints module parameters.
       */
      void printModuleParams() const;

    private:

      // Enumerators

      /**
       * Label tags
       */
      enum {c_LTrack = 0, c_LextHit, c_LbarHit};

      // Private methods

      /**
       * Configure TOP geometry for reconstruction
       */
      void TOPconfigure();

      /**
       * Return MC particle associated with GFTrack
       * @param track pointer to GFTrack
       * @return pointer to MCParticle or NULL
       */
      const MCParticle* getMCParticle(const GFTrack* track);

      /**
       * Return index of TOPBarHit of a given MC patricle or -1
       * @param particle pointer to MCParticle
       * @return index of TOPBarHit or -1
       */
      int getTOPBarHitIndex(const MCParticle* particle);

      /**
       * Return extrapolated tracks
       * @param tracks vector of TOPtracks to return
       * @param chargedStable particle hypothesis used for extrapolation
       */
      void getTracks(std::vector<TOPtrack> & tracks, Const::ChargedStable chargedStable);

      // Module parameters

      std::string m_gfTracksColName;  /**< GF tracks (input) */
      std::string m_extHitsColName;   /**< Ext reconstructed hits (input) */
      std::string m_topDigitColName;  /**< Digitized data (input) */
      std::string m_topLogLColName;   /**< TOP log likelihoods (output) */
      std::string m_barHitColName;    /**< MC particle hit (to set relation to) */
      int m_debugLevel;               /**< debug level */
      double m_minBkgPerQbar;   /**< minimal assumed background photons per bar */
      double m_ScaleN0;         /**< scale factor for N0 */

      // Geometry parameters

      TOPGeometryPar* m_topgp;   /**< geometry parameters */

      // space for TOP bars including wedges

      double m_R1;   /**< inner radius */
      double m_R2;   /**< outer radius */
      double m_Z1;   /**< backward z */
      double m_Z2;   /**< forward z */

      // Masses of particle hypotheses

      enum {c_Nhyp = 5};        /**< number of hypotheses */
      double m_Masses[c_Nhyp];  /**< particle masses */

    };

  } // top namespace
} // Belle2 namespace

#endif // TOPRECONSTRUCTORMODULE_H
