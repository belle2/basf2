/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contacts: Tadeas Bilka and Sergey Yashchenko                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <genfit/KalmanFitter.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/GblFitter.h>
#include <genfit/GFGbl.h>

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
  class GBLfitModule : public Module {

  public:
    /** Constructor .
     */
    GBLfitModule();

    /** Destructor.
     */
    virtual ~GBLfitModule();

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
    std::string m_telHitsColName;                    /**< TelHits collection name */
    std::string m_bklmHitsColName;                   /**< BKLMHits2d collection name */
    std::string m_eklmHitsColName;                   /**< EKLMHits2d collection name */

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

    std::string
    m_filterId;                          /**< Select the filter: 'Kalman' for Kalman, 'DAF for DAF, 'simpleKalman' for the simple Kalman. */

    bool m_useClusters;                              /**< Boolean to mark if PXD/SVD cluster hits should be used instead of true hits in the.*/
    std::vector<int>
    m_pdgCodes;                     /**< holds the PDG codes the user sets. If empty the PDG code from genfit::TrackCand will be written into it*/
    bool m_usePdgCodeFromTrackCand;                  /**< flag to indicate if PDG code will be taken from genfit::TrackCand or from user input in m_pdgCodes*/
    std::ofstream HelixParam;                        /**< Text output file name */
    std::vector<double>
    m_dafTemperatures;           /**< holds the annealing scheme for the DAF. The number of vector elements is the number of DAF iterations */
    std::string
    m_resolveWireHitAmbi;                /**< Determines how the ambiguity of wire measurements should be dealt with.  If this is set to 'default', we use 'weightedAverage' for the DAF is, the Kalman fit uses 'unweightedClosestToReference', and the simple Kalman (which doesn't have a reference) uses 'unweightedClosestToPrediction'. */
    std::vector<double>
    m_beamSpot;                  /**< The coordinates of the point whose POCA will define the parameters of the TrackFitResults.  */
    genfit::GblFitter m_gbl;                         /**< General Broken Line interface class object. */

    std::string m_gblInternalIterations;             /**< GBL internal downweighting setting */
    int m_gblExternalIterations;                     /**< GBL: Real GBL iteration with state updates */
    double m_gblPvalueCut;                           /**< GBL: p-value cut to output track to millepede file */
    int m_gblMinNdf;                                 /**< GBL: minimum NDF to output track to millepede file */
    std::string m_gblMilleFileName;                  /**< GBL: Name of the mille binary file to be produced for alignment */
    double m_chi2Cut;                                /**< GBL: Cut on single measurement Chi2", double(50.)) */
    bool m_enableScatterers;                         /**< GBL: Enable scattering in GBL trajectory", bool(true)) */
    bool m_enableIntermediateScatterer;              /**< GBL: Enable intermediate scatterers for simulation of thick scatterer", bool(true)) */
    bool m_resortHits;                               /**< GBL: Sort hits by extrapolation before init of GBL fit */
    int m_recalcJacobians;                           /**< Recalculate Jacobians: 0=do not recalc, 1=after 1st fit, 2=1+after 2nd fit, etc. */
    bool m_storeFailed;                              /**< Store tracks where the fit failed */
    bool m_useOldGbl;                                /**< Use old GBL interface version */
    bool m_seedFromDAF;                              /**< Prefit track with DAF to get better seed */

    HitPatternCDC getHitPatternCDC(genfit::Track); /**< returns HitPatternCDC of the Track */
    HitPatternVXD getHitPatternVXD(genfit::Track); /**< returns the HitPatternVXD of the Track*/
    std::string
    m_pruneFlags;                        /**< Describes which information to keep after the track fit, see genfit::Track::prune for possible settings. */

    bool m_realisticCDCGeoTranslator;                /**< Whether to use the realistic geometry translators.  */
    bool m_enableWireSag;                            /**< Wire sag in CDCGeometryTranslator.  */
    bool m_useTrackTime;                             /**< If true, CDCRecoHits will use the track propagation time.  */
    bool m_estimateSeedTime;                         /**< If true, the starting time of each track will be estimated from its seed.  */
    bool m_buildBelle2Tracks;                        /**< Create Belle2::Tracks in the DataStore? */
    bool m_addDummyVertexPoint;                       /**< Add point at beginning of the track at position of the seed with "negative" covariance */

  };
}

