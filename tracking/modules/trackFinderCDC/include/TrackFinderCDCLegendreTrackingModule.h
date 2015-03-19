/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackFitter.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackProcessor.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class FastHough;
    class TrackDrawer;
  }


  /** CDC tracking module, using Legendre transformation of the drift time circles.
   * This is a module, performing tracking in the CDC. It is based on the paper "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers" by T. Alexopoulus, et al. NIM A592 456-462 (2008)
   */
  class CDCLegendreTrackingModule: public virtual  Module {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     */
    CDCLegendreTrackingModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     */
    void initialize() override;

    /** Called when entering a new run;
     * Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    void beginRun() override
    {
      eventNumber = 0;
      m_treeFinder = 0;
      m_steppedFinder = 0;
    }

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;

    /** This method is called if the current run ends.
     * Use this method to store information, which should be aggregated over one run.
     */
    void endRun() override;

    /** This method is called at the end of the event processing.
     *  Use this method for cleaning up, closing files, etc.
     */
    void terminate() override;


  protected:

  private:

    const double m_rMin = -0.15; /**< Minimum in r direction*/
    const double m_rMax = 0.15; /**< Maximum in r direction*/
    const int m_nbinsTheta =
      8192; /**< hardcoded value!!! temporary solution, for avoiding segfaults only  //static_cast<int>(std::pow(2.0, m_maxLevel + 3)); //+3 needed for make bin overlapping; */

    TrackFindingCDC::TrackFitter m_cdcLegendreTrackFitter; /**< Object containing fitter for tracking */
    TrackFindingCDC::QuadTreeLegendre m_cdcLegendreQuadTree; /**< Object which holds quadtree structure */
    TrackFindingCDC::TrackProcessor m_cdcLegendreTrackProcessor; /**< Object for creating tracks */

    TrackFindingCDC::FastHough* m_cdcLegendreFastHough; /**< Fast Hough transformer */
    TrackFindingCDC::TrackDrawer* m_cdcLegendreTrackDrawer; /**< Class which allows in-module drawing*/

    std::string m_param_cdcHitsColumnName; /**< Input digitized hits collection name (output of CDCDigitizer module) */
    std::string m_param_trackCandidatesColumnName; /**< Output genfit track candidates collection name*/
    unsigned int m_param_threshold;         /**< Threshold for votes in the legendre plane, parameter of the module*/
    double m_thresholdUnique;     /**< Threshold of unique TrackHits for track building*/
    double m_stepScale;           /**< Scale of steps for SteppedHough*/
    int m_maxLevel;               /**< Maximum Level of FastHough Algorithm*/
    bool m_reconstructCurler;     /**< Stores, curlers shall be reconstructed*/
    bool m_earlyMerge;            /**< Apply fitting for candidates or not*/
    bool m_drawCandidates;        /**< Draw each candidate in interactive mode*/
    bool m_drawCandInfo;          /**< Set whether TrackDrawer class will bw used at all*/
    bool m_deleteHitsWhileFinding;    /**< Try to delete bad hits from track candidate */
    bool m_deleteHitsInTheEnd;        /**< Try to delete bad hits from track candidate in the end */
    bool m_appendHitsInTheEnd;        /**< Try to append new hits to track candidate in the end*/
    bool m_appendHitsWhileFinding;    /**< Try to append new hits to track candidate while finding*/
    bool m_mergeTracksWhileFinding;   /**< Try to merge tracks while finding*/
    bool m_mergeTracksInTheEnd;       /**< Try to merge tracks in the end. */
    bool m_doPostprocessingOften;     /**< Repeat the postprocessing mode after every tree search. */
    int m_treeFindingNumber;          /**< Repeat the whole process that many times. */
    bool m_batchMode;             /**< Sets ROOT to batch mode (don't show canvases) */

    int m_treeFinder;
    int m_steppedFinder;

    int eventNumber;

    /**
     * Function used in the event function, which contains the search for tracks, calling multiply the Fast Hough algorithm, always just searching for one track and afterwards removin the according hits from the hit list.
     */
    void doTreeTrackFinding(unsigned int limit, double rThreshold, bool increaseThreshold = false);

    /**
     * Do the postprocessing like merging, reassignment or deleting of hits.
     */
    void postprocessTracks();

    /**
     * All objects in m_hitList and m_trackList are deleted and the two lists are cleared.
     */
    void clearVectors();

    /**
     * Do the real tree track finding.
     */
    void findTracks();

    /**
     * Startup code before the event starts.
     */
    void startNewEvent();

    /**
     * Write the objects to the store array.
     */
    void outputObjects();

    /**
     * Create candidates using vector of quadtree nodes.
     */
    void processNodes(std::vector<TrackFindingCDC::QuadTreeLegendre*>&, TrackFindingCDC::QuadTreeLegendre::CandidateProcessorLambda&,
                      int);

  };


} // end namespace Belle2


