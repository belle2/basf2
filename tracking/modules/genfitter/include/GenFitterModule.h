/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/DAF.h>

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

    std::string m_tracksColName;                     /**< Tracks collection name */
    std::string m_gfTracksColName;                   /**< genfit::Tracks collection name */
    bool m_suppressGFExceptionOutput;                /**< Switch on/off printing GenFit error message output */

    //These are two different counters, the counter for genfit::TrackCands refers to the number of input genfit::TrackCands. The number of total successfull/failed fits may be different, if genfit::TrackCand is fitted several times under different PDG hypothesises.
    int m_failedFitCounter;                          /**< Number of failed fits. */
    int m_successfulFitCounter;                      /**< Number of successfully fitted tracks. */

    int m_failedGFTrackCandFitCounter;               /**< Number of genfit::TrackCands with failed fits. */
    int m_successfulGFTrackCandFitCounter;           /**< Number of genfit::TracksCands with successfully fitted tracks. */

    std::string m_filterId;                          /**< Select the filter: 'Kalman' for Kalman, 'DAF for DAF, 'simpleKalman' for the simple Kalman. */
    int m_nMinIter;                                  /**< Minimum number of iterations for the Kalman filter. */
    int m_nMaxIter;                                  /**< Maximum number of iterations for the Kalman filter. */
    double m_probCut;                                /**< Probability cut for the DAF filter (0.001, 0.005, 0.01). */

    bool m_storeFailed;                              /**< Boolean to mark if failed track fits should also be stored as Tracks. */
    bool m_useClusters;                              /**< Boolean to mark if PXD/SVD cluster hits should be used instead of true hits in the.*/
    std::vector<int> m_pdgCodes;                     /**< holds the PDG codes the user sets. If empty the PDG code from genfit::TrackCand will be written into it*/
    bool m_usePdgCodeFromTrackCand;                  /**< flag to indicate if PDG code will be taken from genfit::TrackCand or from user input in m_pdgCodes*/
    std::ofstream HelixParam;                        /**< Text output file name */
    bool m_createTextFile;                           /**< Boolean to select if an output file with helix parameters should be created. */
    std::vector<double> m_dafTemperatures;           /**< holds the annealing scheme for the DAF. The number of vector elements is the number of DAF iterations */
    bool m_energyLossBetheBloch;                     /**< Determines if calculation of energy loss is on/off in Genfit */
    bool m_noiseBetheBloch;                          /**< Determines if calculation of energy loss variance is on/off in Genfit */
    bool m_noiseCoulomb;                             /**< Determines if calculation of multiple scattering covariance matrix on/off in Genfit */
    bool m_energyLossBrems;                          /**< Determines if calculation of bremsstrahlung energy loss is on/off in Genfit */
    bool m_noiseBrems;                               /**< Determines if calculation of bremsstrahlung energy loss variance is on/off in Genfit */
    bool m_noEffects;                                /**< switch on/off ALL material effects in Genfit. "true" overwrites "true" flags for the individual effects.*/
    std::string m_mscModel;                          /**< Multiple scattering model */
    std::string m_resolveWireHitAmbi;                /**< Determines how the ambiguity of wire measurements should be dealt with.  If this is set to 'default', we use 'weightedAverage' for the DAF, the Kalman fit uses 'unweightedClosestToReference', and the simple Kalman (which doesn't have a reference) uses 'unweightedClosestToPrediction'. */
    std::vector<double> m_beamSpot;                  /**< The coordinates of the point whose POCA will define the parameters of the TrackFitResults.  */
  };
}

