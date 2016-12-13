/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/DAF.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <fstream>
#include <string>

namespace Belle2 {

  /** This module uses genfit::TrackCand objects (track candidates with hits and momentum seed)
   *  and fits them using GenFit.
   *
   *  This modules can be executed after MCTrackFinderModule or any pattern
   *  recognition module, as long as these modules provide a valid genfit::TrackCand array.
   *  At the end genfit::Tracks (native GenFit output with all the information) and
   *  Tracks (custom objects with only the important parameters we want to have at the end)
   *  are created.
   */
  class GenFitterModule : public Module {

  public:
    /** Constructor .
     */
    GenFitterModule();

    /** Destructor.
     */
    virtual ~GenFitterModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();

  private:

    std::string m_cdcHitsColName;                    /**< CDCHits collection name */
    std::string m_svdHitsColName;                    /**< SVDHits collection name */
    std::string m_pxdHitsColName;                    /**< PXDHits collection name */
    std::string m_gfTrackCandsColName;               /**< genfit::TrackCandidates collection name */
    std::string m_mcParticlesColName;                /**< MCParticles collection name */

    std::string m_gfTracksColName;                   /**< genfit::Tracks collection name */
    bool m_suppressGFExceptionOutput;                /**< Switch on/off printing GenFit error message output */

    //These are two different counters, the counter for genfit::TrackCands refers to the number of input genfit::TrackCands. The number of total successfull/failed fits may be different, if genfit::TrackCand is fitted several times under different PDG hypothesises.
    int m_failedFitCounter;                          /**< Number of failed fits. */
    int m_successfulFitCounter;                      /**< Number of successfully fitted tracks. */

    int m_failedGFTrackCandFitCounter;               /**< Number of genfit::TrackCands with failed fits. */
    int m_successfulGFTrackCandFitCounter;           /**< Number of genfit::TracksCands with successfully fitted tracks. */

    //! Select the track fit algorithm
    /** 'Kalman' for Kalman,
     *  'DAF for DAF,
     *  'simpleKalman' for the simple Kalman.  */
    std::string m_filterId;
    int m_nMinIter;                                  /**< Minimum number of iterations for the Kalman filter. */
    int m_nMaxIter;                                  /**< Maximum number of iterations for the Kalman filter. */
    int m_nMaxFailed;                                /**< Maximum number of failed hits before aborting the fit. */
    double m_probCut;                                /**< Probability cut for the DAF filter (0.001, 0.005, 0.01). */
    /// This is the difference on pvalue between two fit iterations of the DAF procedure which
    /// is used as a early termination criteria of the DAF procedure. This is large on purpose
    /// See https://agira.desy.de/browse/BII-1725 for details
    const double m_dafDeltaPval = 1.0f;

    //! Control track pruning
    /** Describes which information to keep after the track fit, see genfit::Track::prune for possible settings.
     *  Note that the reconstruction script defines a module for this purpose,
     *  which allows this to happen after dE/dx has been performed.  */
    std::string m_pruneFlags;

    bool m_storeFailed;                              /**< Boolean to mark if failed track fits should also be stored as Tracks. */

    bool m_initializeCDC;                            /**< Boolean to signal if the CDC Translators should be initialized. In VXD-only beam test scenarios
                                                          This may be not needded */
    bool m_useClusters;                              /**< Boolean to mark if PXD/SVD cluster hits should be used instead of true hits in the track fit. */
    bool m_realisticCDCGeoTranslator;                /**< Whether to use the realistic geometry translators.  */
    bool m_enableWireSag;                            /**< Wire sag in CDCGeometryTranslator.  */
    bool m_useTrackTime;                             /**< If true, CDCRecoHits will use the track propagation time.  */
    bool m_estimateSeedTime;                         /**< If true, the starting time of each track will be estimated from its seed.  */
    //! PDG codes to process
    /** Holds the PDG codes the user sets.  If empty, the PDG code from genfit::TrackCand will be written into it.  */
    std::vector<int> m_pdgCodes;
    bool m_usePdgCodeFromTrackCand;                  /**< flag to indicate if PDG code will be taken from genfit::TrackCand or from user input in m_pdgCodes*/

    //! Annealing scheme for the DAF.
    /** This is a list of temperatures, applied in sequence.  The DAF always processes the whole list.  */
    std::vector<double> m_dafTemperatures;

    //! How to resolve the wire ambiguity
    /** Determines how the ambiguity of wire measurements should be dealt with.
     *  If this is set to 'default', we use 'weightedAverage' for the DAF,
     *  the Kalman fit uses 'unweightedClosestToReference',
     *  and the simple Kalman (which doesn't have a reference) uses 'unweightedClosestToPrediction.
     */
    std::string m_resolveWireHitAmbi;
    //! Beamspot coordinates
    std::vector<double> m_beamSpot;
  };
}

