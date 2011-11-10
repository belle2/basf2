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

namespace Belle2 {


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

    std::string m_tracksColName;                     /**< Tracks collection name */
    std::string m_gfTracksColName;                   /**< GFTracks collection name */

    int m_failedFitCounter;                          /**< Number of failed fits. */
    int m_successfulFitCounter;                      /**< Number of successfully fitted tracks. */

    bool m_mcTracks;                                 /**< Boolean to mark if MCTrack or pattern reco tracks should be fitted. */

    int m_filterId;                                  /**< Select the filter: 0 for Kalman, 1 (>0) for DAF. */
    int m_nIter;                                     /**< Number of iterations for the Kalman filter. */
    double m_probCut;                                /**< Probability cut for the DAF filter (0.001, 0.005, 0.01). */

    bool m_storeFailed;                              /**< Boolean to mark if failed track fits should also be stored as Tracks. */
    int m_pdg;                                       /**< PDG hypothesis for the track fit. If -999: MC/default values will be used. */

    std::ofstream HelixParam;                        /**< Text output file name */
    bool m_createTextFile;                           /**< Boolean to select if an output file with helix parameters should be created. */


  };
} // end namespace Belle2


#endif /* GENFITTERMODULE_H */
