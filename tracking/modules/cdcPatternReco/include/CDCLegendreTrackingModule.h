/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <fstream>

#include <boost/tuple/tuple.hpp>
#include <vector>


#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCreator.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackMerger.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>
#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackDrawer.h>
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTree.h>
#include <tracking/cdcLegendreTracking/CDCLegendreConformalPosition.h>
#include <tracking/cdcLegendreTracking/CDCLegendreQuadTreeCandidateCreator.h>
#include "tracking/cdcLegendreTracking/CDCLegendreQuadTreeNeighborFinder.h"
#include <tracking/cdcLegendreTracking/CDCLegendreWireCenter.h>

#include <tracking/cdcLegendreTracking/CDCLegendreSimpleFilter.h>

namespace Belle2 {

  /** Forward declaration to avoid including the corresponding header file
  class TrackHit;
  class TrackCandidate;
  class TrackFitter;
  class PatternChecker;
  class FastHough;
  class TrackMerger;
  class TrackCreator;
  class TrackDrawer;
  class QuadTree;
  class ConformalPosition;
  class QuadTreeCandidateCreator;
  */
  class CDCNiceDrawingModule;

  /** CDC tracking module, using Legendre transformation of the drift time circles.
   * This is a module, performing tracking in the CDC. It is based on the paper "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers" by T. Alexopoulus, et al. NIM A592 456-462 (2008)
   */
  class CDCLegendreTrackingModule: public Module {

  public:

    /** Constructor.
     *  Create and allocate memory for variables here. Add the module parameters in this method.
     */
    CDCLegendreTrackingModule();

    /** Destructor.
     * Use the destructor to release the memory you allocated in the constructor.
     */
    virtual ~CDCLegendreTrackingModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     */
    virtual void initialize();

    /** Called when entering a new run;
     * Called at the beginning of each run, the method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     * Use this method to store information, which should be aggregated over one run.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     *  Use this method for cleaning up, closing files, etc.
     */
    virtual void terminate();


  protected:

  private:

    std::string m_cdcHitsColName; /**< Input digitized hits collection name (output of CDCDigitizer module) */
    std::string m_gfTrackCandsColName; /**< Output genfit track candidates collection name*/

    std::vector<TrackFinderCDCLegendre::TrackHit*> m_AxialHitList; /**< List of the axial hits used for track finding. This is the vector, which is used for memory management! */
    std::vector<TrackFinderCDCLegendre::TrackHit*> m_StereoHitList; /**< List of the stereo hits used for track finding. This is the vector, which is used for memory management! */
    std::list<TrackFinderCDCLegendre::TrackCandidate*> m_trackList; /**< List of track candidates. Mainly used for memory management! */
    std::list<TrackFinderCDCLegendre::TrackCandidate*> m_trackletList; /**< List of tracklets. */
    std::list<TrackFinderCDCLegendre::TrackCandidate*> m_stereoTrackletList; /**< List of tracklets. */

    TrackFinderCDCLegendre::TrackFitter* m_cdcLegendreTrackFitter; /**< Object containing fitter for tracking */
    TrackFinderCDCLegendre::PatternChecker* m_cdcLegendrePatternChecker; /**< Used for check patterns of tracks */
    TrackFinderCDCLegendre::FastHough* m_cdcLegendreFastHough; /**< Fast Hough transformer */
    TrackFinderCDCLegendre::TrackMerger* m_cdcLegendreTrackMerger; /**< Object which make track merging inside the module */
    TrackFinderCDCLegendre::TrackCreator* m_cdcLegendreTrackCreator; /**< Object for creating tracks */
    TrackFinderCDCLegendre::QuadTree* m_cdcLegendreQuadTree; /**< Object which holds quadtree structure */
    TrackFinderCDCLegendre::ConformalPosition* m_cdcLegendreConformalPosition; /**< Object which holds pre-calculated conformal transformation of each wire */
    TrackFinderCDCLegendre::QuadTreeCandidateCreator* m_cdcLegendreQuadTreeCandidateCreator; /**< Object which creates track candidates using quadtree nodes */

    int m_threshold; /**< Threshold for votes in the legendre plane, parameter of the module*/
    double m_thresholdUnique; /**< Threshold of unique TrackHits for track building*/
    double m_stepScale; /**< Scale of steps for SteppedHough*/
    int m_initialAxialHits; /**< Initial number of axial hits in the stepped hough algorithm*/
    int m_nbinsTheta; /**< Number of bins in theta, derived from m_maxLevel*/

    double m_resolutionStereo; /**< Total resolution, used for the assignment of stereo hits to tracks*/

    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/

    int m_maxLevel; /**< Maximum Level of FastHough Algorithm*/

    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/

    bool m_reconstructCurler; /**< Stores, curlers shall be reconstructed*/
    bool m_fitTracks; /**< Apply fitting for candidates or not*/
    bool m_fitTracksEarly; /**< Apply fitting for candidates on early stage or not*/
    bool m_mergeTracksEarly; /**< Apply fitting for candidates on early stage or not*/
    bool m_earlyMerge; /**< Apply fitting for candidates or not*/
    bool m_drawCandidates; /**< Draw each candidate in interactive mode*/
    bool m_drawCandInfo; /**< Set whether TrackDrawer class will bw used at all*/
    bool m_appendHits; /**< Try to append new hits to track candidate*/
    bool m_multipleCandidateSearch; /**< Search multiple track candidates per run of FastHough algorithm*/
    bool m_useHitPrecalculatedR; /**< To store r values inside hit objects or recalculate it each step */

    TrackFinderCDCLegendre::TrackDrawer* m_cdcLegendreTrackDrawer; /**< Class which allows in-module drawing*/

    int m_treeFinder;
    int m_steppedFinder;
    /**
     * Function used in the event function, which contains the search for tracks, calling multiply the Fast Hough algorithm, always just searching for one track and afterwards removin the according hits from the hit list.
     */
    void DoSteppedTrackFinding();

    /**
     * Function used in the event function, which contains the search for tracks, calling multiply the Fast Hough algorithm, always just searching for one track and afterwards removin the according hits from the hit list.
     */
    void DoTreeTrackFinding();

    /**
     * Finding of stereo tracklets which will be assigned to some track
     */
    void DoTreeStereoTrackletFinding();

    /**
     * In this function, the stereo hits are assigned to the track candidates.
     */
    void AsignStereoHits();


    /**
     * @brief Function to split track candidates into 3 groups: long tracks, curlers, and tracklets
     * Long tracks - passed through all superlayers; curlers - starts at 1st superlayer, but not reach 9th; tracklets - some kind of cluster somewhere in CDC, should be merged with other tracklets or tracks
     */
    void checkHitPattern();



    /**
     * @brief Function to improve quality of founded tracks
     */
    void processTracks();



    /**
     * All objects in m_hitList and m_trackList are deleted and the two lists are cleared.
     * Necessary since we cannot use smart pointers up to now.
     */
    void clear_pointer_vectors();

  };


} // end namespace Belle2


