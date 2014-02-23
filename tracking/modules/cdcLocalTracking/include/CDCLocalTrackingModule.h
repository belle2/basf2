/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCLOCALTRACKINGMODULE_H_
#define CDCLOCALTRACKINGMODULE_H_

//#define LOG_NO_B2DEBUG
#include <framework/core/Module.h>

#include <tracking/cdcLocalTracking/config/CDCLocalTrackingConfig.h>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/workers/FacetSegmentWorker.h>

#include <tracking/cdcLocalTracking/filters/facet/SimpleFacetFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/facet/MCFacetFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/MCFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/facet/OptimizingFacetFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/OptimizingFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/workers/SegmentTripleTrackingWorker.h>

#include <tracking/cdcLocalTracking/filters/axial_axial/SimpleAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/cdcLocalTracking/filters/axial_axial/EvaluateAxialAxialSegmentPairFilter.h>

#include <tracking/cdcLocalTracking/filters/segment_triple/SimpleSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleSegmentTripleNeighborChooser.h>

#include <tracking/cdcLocalTracking/filters/segment_triple/MCSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/MCSegmentTripleNeighborChooser.h>

namespace Belle2 {

  class CDCLocalTrackingModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    CDCLocalTrackingModule();

    /** Destructor of the module.
     */
    virtual ~CDCLocalTrackingModule();

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

    /** End of the event processing.
     */
    virtual void terminate();

  protected:

  private:

    std::string m_param_gfTrackCandColName; /**< Name of the output collection of genfit::TrackCands */


#ifdef CDCLOCALTRACKING_USE_MC_FILTERS
#ifdef CDCLOCALTRACKING_USE_OPTIMIZING_FILTERS
    typedef CDCLocalTracking::OptimizingFacetFilter FacetFilter;
    typedef CDCLocalTracking::OptimizingFacetNeighborChooser FacetNeighborChooser;
#else
    typedef CDCLocalTracking::MCFacetFilter FacetFilter;
    typedef CDCLocalTracking::MCFacetNeighborChooser FacetNeighborChooser;
#endif

#else
    typedef CDCLocalTracking::SimpleFacetFilter FacetFilter;
    typedef CDCLocalTracking::SimpleFacetNeighborChooser FacetNeighborChooser;
#endif

    CDCLocalTracking::FacetSegmentWorker<FacetFilter, FacetNeighborChooser> m_segmentWorker;
    //storing the recosegments for the whole event
    std::vector< Belle2::CDCLocalTracking::CDCRecoSegment2D > m_recoSegments;


#ifdef CDCLOCALTRACKING_USE_MC_FILTERS
#ifdef CDCLOCALTRACKING_USE_OPTIMIZING_FILTERS
    //No optimizing filters and choosers for segment triples yet.
    typedef CDCLocalTracking::EvaluateAxialAxialSegmentPairFilter <
    CDCLocalTracking::SimpleAxialAxialSegmentPairFilter
    > AxialAxialSegmentPairFilter;

    typedef CDCLocalTracking::MCSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::MCSegmentTripleNeighborChooser SegmentTripleNeighborChooser;
#else
    typedef CDCLocalTracking::EvaluateAxialAxialSegmentPairFilter <
    CDCLocalTracking::SimpleAxialAxialSegmentPairFilter
    > AxialAxialSegmentPairFilter;
    //typedef CDCLocalTracking::MCAxialAxialSegmentPairFilter AxialAxialSegmentPairFilter;

    typedef CDCLocalTracking::MCSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::MCSegmentTripleNeighborChooser SegmentTripleNeighborChooser;
#endif

#else
    typedef CDCLocalTracking::SimpleAxialAxialSegmentPairFilter AxialAxialSegmentPairFilter;
    typedef CDCLocalTracking::SimpleSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::SimpleSegmentTripleNeighborChooser SegmentTripleNeighborChooser;
#endif


    CDCLocalTracking::SegmentTripleTrackingWorker< AxialAxialSegmentPairFilter, SegmentTripleFilter, SegmentTripleNeighborChooser> m_trackingWorker;


  }; // end class
} // end namespace Belle2

#endif /* CDCLOCALTRACKINGMODULE_H_ */

