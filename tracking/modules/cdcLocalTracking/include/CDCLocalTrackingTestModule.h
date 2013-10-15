/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCLOCALTRACKINGTESTMODULE_H_
#define CDCLOCALTRACKINGTESTMODULE_H_

//#define LOG_NO_B2DEBUG

#include <framework/core/Module.h>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include <tracking/cdcLocalTracking/creators/WireHitCreator.h> //no decisions to optimize

#include <tracking/cdcLocalTracking/workers/FacetSegmentWorker.h>

#include <tracking/cdcLocalTracking/creator_filters/SimpleFacetFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/creator_filters/MCFacetFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/MCFacetNeighborChooser.h>

#include <tracking/cdcLocalTracking/workers/SegmentTripleTrackingWorker.h>

#include <tracking/cdcLocalTracking/creator_filters/SimpleSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/SimpleSegmentTripleNeighborChooser.h>

#include <tracking/cdcLocalTracking/creator_filters/MCSegmentTripleFilter.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/MCSegmentTripleNeighborChooser.h>


#include <tracking/cdcLocalTracking/geometry/GeneralizedCircle.h>
#include <tracking/cdcLocalTracking/geometry/Line2D.h>
#include <tracking/cdcLocalTracking/geometry/ParameterLine2D.h>

namespace Belle2 {

  class CDCLocalTrackingTestModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    CDCLocalTrackingTestModule();

    /** Destructor of the module.
     */
    virtual ~CDCLocalTrackingTestModule();

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

    void test_modulo() const;
    void test_sizeof() const;
    void test_mock_root() const;
    void test_nullptr() const;
    void test_function_templates() const;
    void test_size_of_dataholders() const;
    void test_generalized_circle() const;
    void batch_line2d() const;
    void test_line2d(const CDCLocalTracking::Line2D& line) const;
    void test_parameter_line2d(const CDCLocalTracking::ParameterLine2D& line) const;
    void test_wire_topology() const;
    void test_enums() const;
    void test_wire_neighbor_symmetry() const;
    void test_recotangent() const;

  private:

    //std::string m_inColName; /**< Input digitized hits collection name (output of CDCDigitizer module) */
    /*
    CDCLocalTracking::WireHitCreator m_wirehitCreator;

    //typedef CDCLocalTracking::SimpleFacetFilter FacetFilter;
    typedef CDCLocalTracking::MCFacetFilter FacetFilter;

    //typedef CDCLocalTracking::SimpleFacetNeighborChooser FacetNeighborChooser;
    typedef CDCLocalTracking::MCFacetNeighborChooser FacetNeighborChooser;

    CDCLocalTracking::FacetSegmentWorker<FacetFilter,FacetNeighborChooser> m_segmentWorker;


    //storing the recosegments for the whole event
    std::vector< Belle2::CDCLocalTracking::CDCRecoSegment2D > m_recoSegments;


    //typedef CDCLocalTracking::SimpleSegmentTripleFilter SegmentTripleFilter;
    typedef CDCLocalTracking::MCSegmentTripleFilter SegmentTripleFilter;

    //typedef CDCLocalTracking::SimpleSegmentTripleNeighborChooser SegmentTripleChooser;
    typedef CDCLocalTracking::MCSegmentTripleNeighborChooser SegmentTripleChooser;

    CDCLocalTracking::SegmentTripleTrackingWorker<SegmentTripleFilter,SegmentTripleChooser> m_trackingWorker;
    */

  }; // end class
} // end namespace Belle2

#endif /* CDCLOCALTRACKINGTESTMODULE_H_ */

