/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    class FastHough;
    class CDCTrack;
  }


  /** CDC tracking module, using Legendre transformation of the drift time circles.
   * This is a module, performing tracking in the CDC. It is based on the paper "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers" by T. Alexopoulus, et al. NIM A592 456-462 (2008)
   */
  class CDCLegendreTrackingModule: virtual public TrackFinderCDCBaseModule {

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

    /** This method is called at the end of the event processing.
     *  Use this method for cleaning up, closing files, etc.
     */
    void terminate() override;


  protected:

  private:

    const double m_rMin = 0.; /**< Minimum in r direction*/
    const double m_rMax = 0.075; /**< Maximum in r direction*/
    const unsigned long m_nbinsTheta =
      pow(2, 16); /**< hardcoded value!!! temporary solution, for avoiding segfaults only  //static_cast<int>(std::pow(2.0, m_maxLevel + 3)); //+3 needed for make bin overlapping; */

    std::vector<TrackFindingCDC::CDCRecoSegment2D> m_segments;


    TrackFindingCDC::TrackProcessorNew m_trackProcessor; /**< Object for creating tracks */

    unsigned int m_param_threshold;         /**< Threshold for votes in the legendre plane, parameter of the module*/
    double m_param_stepScale;           /**< Scale of steps for SteppedHough*/
    int m_maxLevel;               /**< Maximum Level of FastHough Algorithm*/
    bool m_deleteHitsWhileFinding;    /**< Try to delete bad hits from track candidate */
    bool m_deleteHitsInTheEnd;        /**< Try to delete bad hits from track candidate in the end */
    bool m_appendHitsInTheEnd;        /**< Try to append new hits to track candidate in the end*/
    bool m_appendHitsWhileFinding;    /**< Try to append new hits to track candidate while finding*/
    bool m_mergeTracksWhileFinding;   /**< Try to merge tracks while finding*/
    bool m_mergeTracksInTheEnd;       /**< Try to merge tracks in the end. */
    bool m_doPostprocessingOften;     /**< Repeat the postprocessing mode after every tree search. */
    bool m_increasePerformance;       /**< Repeat the postprocessing mode after every tree search. */
    int m_treeFindingNumber;          /**< Repeat the whole process that many times. */
    bool m_batchMode;                 /**< Sets ROOT to batch mode (don't show canvases) */

    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

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
    void outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);


    void postprocessSingleNode(std::vector<TrackFindingCDC::QuadTreeHitWrapper*>&, bool,
                               TrackFindingCDC::AxialHitQuadTreeProcessor::QuadTree*);


  };


} // end namespace Belle2


