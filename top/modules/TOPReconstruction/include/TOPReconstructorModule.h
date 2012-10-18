/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
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


namespace Belle2 {
  namespace TOP {

    //! TOP reconstruction module.
    /*!
    */
    class TOPReconstructorModule : public Module {

    public:

      //! Constructor.
      TOPReconstructorModule();

      //! Destructor.
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
       *Prints module parameters.
       */
      void printModuleParams() const;

    private:

      //! Module parameters
      std::string m_gfTracksColName;      /**< GF tracks (input) */
      std::string m_extTrackCandsColName; /**< Ext track candidates (input) */
      std::string m_extRecoHitsColName;   /**< Ext reconstructed hits (input) */
      std::string m_topDigitColName;      /**< Digitized data (input) */
      std::string m_topLogLColName;       /**< TOP log likelihoods (output) */
      std::string m_barHitColName;      /**< MC particle hit (to set relation to) */
      int m_debugLevel; /**< debug level */
      double m_minBkgPerQbar;  /**< minimal assumed background photons per bar */
      double m_ScaleN0;  /**< scale factor for N0 */

      //! Geometry parameters object
      TOPGeometryPar* m_topgp;

      //! space for TOP bars including wedges
      double m_R1; /**< inner radius */
      double m_R2; /**< outer radius */
      double m_Z1; /**< backward z */
      double m_Z2; /**< forward z */

      //! TOP configure function
      void TOPconfigure();

      //! Masses of particle hypotheses
      enum {Nhyp = 5};        /**< number of hypotheses */
      double m_Masses[Nhyp];  /**< particle masses */

      //! Label tags
      enum {LgfTrack = 0, LextTrackCand, LextHit, LbarHit};

      //! MC particle associated with GF track
      const MCParticle* getMCParticle(const GFTrack* track);

      //! index of TOPBarHit of a given MC patricle or -1
      int getTOPBarHitIndex(const MCParticle* particle);

      //! get extrapolated tracks
      void getTracks(std::vector<TOPtrack> & tracks, int hypothesis);

    };

  } // top namespace
} // Belle2 namespace

#endif // TOPRECONSTRUCTORMODULE_H
