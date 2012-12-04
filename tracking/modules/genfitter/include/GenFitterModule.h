/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef GENFITTERMODULE_H
#define GENFITTERMODULE_H

#include <framework/core/Module.h>

#include <fstream>
#include <string>

#include <GFKalman.h>
#include <GFDaf.h>

namespace Belle2 {

  /**
   *  \addtogroup modules
   *  @{
   *  \addtogroup tracking_modules
   *  \ingroup modules
   *  @{ GenFitterModule @} @}
   */


  /** This module use GFTrackCand objects (track candidates with hits and momentum seed) and fits them using GenFit.
   *
   *  This modules can be executed after MCTrackFinderModule or any pattern recognition module, as long as these modules provide valid GFTrackCand array.
   *
   *  At the end GFTracks (native GenFit output with all the information) and Tracks (custom objects with only the important parameters we want to have at the end) are created.
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


  protected:


  private:

    std::string m_cdcHitsColName;                    /**< CDCHits collection name */
    std::string m_svdHitsColName;                    /**< SVDHits collection name */
    std::string m_pxdHitsColName;                    /**< PXDHits collection name */
    std::string m_gfTrackCandsColName;               /**< GFTrackCandidates collection name */
    std::string m_mcParticlesColName;                /**< MCParticles collection name */

    std::string m_tracksColName;                     /**< Tracks collection name */
    std::string m_gfTracksColName;                   /**< GFTracks collection name */

    //These are two different counters, the counter for GFTrackCands refers to the number of input GFTrackCands. The number of total successfull/failed fits may be different, if GFTrackCand is fitted several times under different PDG hypothesises.
    int m_failedFitCounter;                          /**< Number of failed fits. */
    int m_successfulFitCounter;                      /**< Number of successfully fitted tracks. */

    int m_failedGFTrackCandFitCounter;               /**< Number of GFTrackCands with failed fits. */
    int m_successfulGFTrackCandFitCounter;           /**< Number of GFTracksCands with successfully fitted tracks. */

    int m_filterId;                                  /**< Select the filter: 0 for Kalman, 1 (>0) for DAF. */
    int m_nIter;                                     /**< Number of iterations for the Kalman filter. */
    double m_probCut;                                /**< Probability cut for the DAF filter (0.001, 0.005, 0.01). */

    bool m_storeFailed;                              /**< Boolean to mark if failed track fits should also be stored as Tracks. */
    bool m_useClusters;                              /**< Boolean to mark if PXD/SVD cluster hits should be used instead of true hits in the.*/
    std::vector<int> m_pdgCodes;                     /**< holds the PDG codes the user sets. If empty the PDG code from GFTrackCand will be written into it*/
    bool m_usePdgCodeFromTrackCand;                  /**< flag to indicate if PDG code will be taken from GFTrackCand or from user input in m_pdgCodes*/
    std::ofstream HelixParam;                        /**< Text output file name */
    bool m_createTextFile;                           /**< Boolean to select if an output file with helix parameters should be created. */
    GFKalman m_kalmanFilter;                         /**< The Genfit Kalman filter object */
    GFDaf m_daf;                                     /**< The Genfit Deterministic Annealing filter (DAF) object */
    std::vector<double> m_dafTemperatures;           /**< holds the annealing scheme for the DAF. The number of vector elements is the number of DAF iterations */

  }; /** @} @} */
} // end namespace Belle2


#endif /* GENFITTERMODULE_H */

