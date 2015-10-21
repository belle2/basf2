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
#include <tracking/trackFindingCDC/legendre/TrackHolder.h>
#include <tracking/trackFindingCDC/legendre/HitFactory.h>

#include <tracking/trackFindingCDC/legendre/quadtreetools/QuadTreePassCounter.h>

namespace Belle2 {

  namespace TrackFindingCDC {
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

    TrackFindingCDC::QuadTreePassCounter m_QuadTreePassCounter;

    TrackFindingCDC::HitFactory m_hitFactory; /**< Object for creating tracks */
    TrackFindingCDC::TrackHolder m_trackHolder; /**< Object for creating tracks */
    TrackFindingCDC::TrackProcessor m_trackProcessor; /**< Object for creating tracks */

    int m_maxLevel;               /**< Maximum Level of FastHough Algorithm*/
    bool m_doEarlyMerging;               /**< Defines whether early track merging will be preformed*/

    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

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


  };


} // end namespace Belle2


